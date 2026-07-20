#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
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

sim::Engine make_engine(const sim::LockMode lock, std::mt19937_64& rng,
                        sim::State state = known_route_state()) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario scenario{"issue-1186", sim::DciProfile::StrictJit,
                               lock, false, 5};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
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
  for (const sim::LockMode lock : {sim::LockMode::None, sim::LockMode::FullItem,
                                   sim::LockMode::FullRuleBoxAbility,
                                   sim::LockMode::FullCombined}) {
    std::mt19937_64 rng{118600 + static_cast<unsigned>(lock)};
    sim::Engine engine = make_engine(lock, rng);
    expect(sim::EngineTestAccess::known_burnet_route(engine),
           "Unrelated Item and Rule Box Ability locks must not suppress the route.");
  }

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
  test_lock_scope_and_controls();
  test_seed_101_reserves_vstar_and_burnet();
  test_rulebox_seed_101_reserves_vstar_and_burnet();
  return 0;
}
