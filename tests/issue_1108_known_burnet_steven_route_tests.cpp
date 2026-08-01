#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool known_burnet_route(const Engine& engine) {
    return engine.known_burnet_t3_steven_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

bool contains_line(const sim::TraceLog& trace, const std::string& first,
                   const std::string& second) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&first, &second](const std::string& line) {
                       return line.find(first) != std::string::npos &&
                           line.find(second) != std::string::npos;
                     });
}

sim::State known_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 3, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::StevensResolve, sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite};
  return state;
}

const sim::Scenario& scenario_for_lock(const sim::LockMode lock) {
  static const sim::Scenario none{"issue-1186-none", sim::DciProfile::StrictJit,
                                  sim::LockMode::None, false, 5};
  static const sim::Scenario turn_two_item{
      "issue-1186-turn-two-item", sim::DciProfile::StrictJit,
      sim::LockMode::TurnTwoItem, false, 5};
  static const sim::Scenario full_item{
      "issue-1186-full-item", sim::DciProfile::StrictJit,
      sim::LockMode::FullItem, false, 5};
  static const sim::Scenario rulebox{
      "issue-1186-rulebox", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  static const sim::Scenario combined{
      "issue-1186-combined", sim::DciProfile::StrictJit,
      sim::LockMode::FullCombined, false, 5};
  // FullSupporter is a supported production lock and canonical matrix scenario:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_002.inc
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv
  // Confirmed helper defect: https://github.com/FlareZ123/pokemon-sims/issues/2029
  static const sim::Scenario full_supporter{
      "issue-2029-full-supporter", sim::DciProfile::StrictJit,
      sim::LockMode::FullSupporter, false, 5};

  switch (lock) {
    case sim::LockMode::None: return none;
    case sim::LockMode::TurnTwoItem: return turn_two_item;
    case sim::LockMode::FullItem: return full_item;
    case sim::LockMode::FullRuleBoxAbility: return rulebox;
    case sim::LockMode::FullCombined: return combined;
    // Supported enum mapping: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_002.inc
    // Confirmed helper defect: https://github.com/FlareZ123/pokemon-sims/issues/2029
    case sim::LockMode::FullSupporter: return full_supporter;
  }
  throw std::invalid_argument("Unsupported lock mode");
}

sim::Engine make_engine(const sim::LockMode lock, std::mt19937_64& rng,
                        sim::State state = known_route_state(),
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario_for_lock(lock), recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, std::move(state), deck_seen, prizes_revealed);
  return engine;
}

void expect_lock_route(const sim::LockMode lock, const std::uint64_t seed,
                       const char* message) {
  std::mt19937_64 rng{seed};
  sim::Engine engine = make_engine(lock, rng);
  expect(sim::EngineTestAccess::known_burnet_route(engine), message);
}

void test_k1_provenance_and_k0_boundary() {
  // A legal deck search and a complete Hisuian Heavy Ball Prize inspection both
  // establish exact fixed-list K1, while true K0 must remain rejected:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Official Prize, Supporter, evolution, search, shuffle, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Existing route regression: https://github.com/FlareZ123/pokemon-sims/issues/1108
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2021
  std::mt19937_64 deck_rng{202101};
  sim::Engine deck_k1 = make_engine(
      sim::LockMode::None, deck_rng, known_route_state(), true, false);
  expect(sim::EngineTestAccess::known_burnet_route(deck_k1),
         "The deck-search K1 Steven-Burnet route was rejected.");

  std::mt19937_64 prize_rng{202102};
  sim::Engine prize_k1 = make_engine(
      sim::LockMode::None, prize_rng, known_route_state(), false, true);
  expect(sim::EngineTestAccess::known_burnet_route(prize_k1),
         "The Prize-inspection K1 Steven-Burnet route was rejected.");

  std::mt19937_64 k0_rng{202103};
  sim::Engine k0 = make_engine(
      sim::LockMode::None, k0_rng, known_route_state(), false, false);
  expect(!sim::EngineTestAccess::known_burnet_route(k0),
         "The Steven-Burnet selector used exact hidden composition before K1.");
}

void test_full_supporter_mapping() {
  // The focused helper must accept every supported LockMode value, including the
  // canonical FullSupporter scenario, instead of reaching its invalid fallback:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_002.inc
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv
  // https://github.com/FlareZ123/pokemon-sims/issues/2029
  const sim::Scenario& scenario = scenario_for_lock(sim::LockMode::FullSupporter);
  expect(scenario.locks == sim::LockMode::FullSupporter,
         "The focused helper did not preserve the FullSupporter lock mode.");
  expect(scenario.label == "issue-2029-full-supporter",
         "The focused helper did not return its FullSupporter scenario.");
}

void test_lock_scope_and_controls() {
  // Steven's Resolve, ordinary evolution, and Professor Burnet use no Item or
  // Rule Box Pokémon Ability. The modeled Item, Rule Box Ability, and combined
  // locks therefore leave this exact K1 route legal:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1186
  expect_lock_route(sim::LockMode::None, 118600,
                    "The known route must remain available without a lock.");
  expect_lock_route(sim::LockMode::FullItem, 118602,
                    "Full Item lock must not suppress the Supporter-evolution route.");
  expect_lock_route(sim::LockMode::FullRuleBoxAbility, 118603,
                    "Rule Box Ability lock must not suppress the Supporter-evolution route.");
  expect_lock_route(sim::LockMode::FullCombined, 118604,
                    "Combined Item and Rule Box Ability lock must not suppress the Supporter-evolution route.");

  sim::State missing_vstar = known_route_state();
  missing_vstar.deck.erase(missing_vstar.deck.begin());
  std::mt19937_64 vstar_rng{118610};
  sim::Engine no_vstar = make_engine(sim::LockMode::FullRuleBoxAbility,
                                     vstar_rng, std::move(missing_vstar));
  expect(!sim::EngineTestAccess::known_burnet_route(no_vstar),
         "The route must require a known searchable VSTAR.");

  sim::State late_active = known_route_state();
  late_active.active->entered_turn = 2;
  std::mt19937_64 timing_rng{118611};
  sim::Engine no_window = make_engine(sim::LockMode::FullItem,
                                      timing_rng, std::move(late_active));
  expect(!sim::EngineTestAccess::known_burnet_route(no_window),
         "The route must preserve the ordinary evolution window.");
}

void test_seed_101_reserves_vstar_and_burnet() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{101};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Steven reserves the known Evolution and direct deck-to-discard Supporter on T2.
  // Burnet then establishes the strict-JIT payload on the same T3 that V evolves:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1108
  expect(outcome.first_ready_turn == 3,
         "Seed 101 must reach strict-JIT readiness on T3.");
  expect(contains(trace, "known T3 Burnet route"),
         "The T2 trace must reserve VSTAR and Professor Burnet.");
  expect(contains_line(trace, "PLAY SUPPORTER", "Professor Burnet"),
         "The T3 trace must play Professor Burnet.");
  expect(!contains(trace, "exchanged Gladion for Regidrago V"),
         "Gladion must not preempt the known T3 completion.");
}

void test_rulebox_seed_101_reserves_vstar_and_burnet() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-second");
  if (!scenario) throw std::runtime_error("Missing Rule Box lock scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{101};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Rule Box Ability lock suppresses modeled Rule Box Abilities. This route uses
  // only Steven, ordinary evolution, and Burnet, so it must still reach T3:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // https://github.com/FlareZ123/pokemon-sims/issues/1186
  expect(outcome.first_ready_turn == 3,
         "Rule Box Ability-lock seed 101 must become ready on T3.");
  expect(contains(trace, "known T3 Burnet route"),
         "Steven must reserve the VSTAR and Professor Burnet under the lock.");
  expect(contains_line(trace, "PLAY SUPPORTER", "Professor Burnet"),
         "Professor Burnet must complete the locked-scenario route.");
  expect(!contains(trace, "exchanged Gladion for Regidrago V"),
         "Gladion must not preempt the lock-independent continuation.");
}

}  // namespace

int main() {
  test_k1_provenance_and_k0_boundary();
  test_full_supporter_mapping();
  test_lock_scope_and_controls();
  test_seed_101_reserves_vstar_and_burnet();
  test_rulebox_seed_101_reserves_vstar_and_burnet();
  return 0;
}
