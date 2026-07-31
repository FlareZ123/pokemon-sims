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
                        const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool complete(Engine& engine) {
    return engine.complete_issue_1596_turo_vessel_dialga_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_seed(const std::string& deck_id,
                    const std::string& scenario_label,
                    const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id(deck_id);
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1596 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

const sim::DeckRecipe& registered_pineco_recipe() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (deck == nullptr) {
    throw std::runtime_error("The registered regidrago-pineco recipe is unavailable.");
  }
  return deck->recipe;
}

sim::Scenario exact_scenario() {
  return sim::Scenario{"issue-2000", sim::DciProfile::MatchupFlexJit,
                       sim::LockMode::None, false, 3};
}

sim::State exact_t3_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::EarthenVessel,
                sim::Card::ChaoticSwell, sim::Card::Gladion};
  state.deck = {sim::Card::Fire, sim::Card::Grass,
                sim::Card::RegidragoV, sim::Card::QuickBall};
  return state;
}

void test_seed_26_plays_turo_before_vessel() {
  const SeedResult result =
      run_seed("regidrago-pineco", "matchup-flex-jit/go-second", 26);
  // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1596
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Pineco seed 26 did not reach readiness on turn three.");
  expect(trace_contains(result.trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(result.trace,
                            "Professor Turo returned Active Dialga-GX") &&
             trace_contains(result.trace, "Dialga-GX (Earthen Vessel cost)") &&
             trace_contains(result.trace, "T3 | READY"),
         "Seed 26 did not preserve the Turo-before-Vessel route.");
}

void test_both_k1_provenances_and_k0_boundary() {
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe& recipe = registered_pineco_recipe();

  // A legal deck search and a complete Hisuian Heavy Ball Prize inspection both
  // establish K1 under the fixed-list policy, while true K0 must remain rejected:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Official Prize, Supporter, return, Item, discard, search, promotion, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2000
  std::mt19937_64 deck_rng{20000};
  sim::Engine deck_k1(scenario, recipe, deck_rng);
  sim::EngineTestAccess::set_state(deck_k1, exact_t3_state(), true, false);
  expect(sim::EngineTestAccess::complete(deck_k1),
         "The deck-search K1 Turo-Vessel route was rejected.");

  std::mt19937_64 prize_rng{20001};
  sim::Engine prize_k1(scenario, recipe, prize_rng);
  sim::EngineTestAccess::set_state(prize_k1, exact_t3_state(), false, true);
  expect(sim::EngineTestAccess::complete(prize_k1),
         "The Prize-inspection K1 Turo-Vessel route was rejected.");

  std::mt19937_64 k0_rng{20002};
  sim::Engine k0(scenario, recipe, k0_rng);
  sim::EngineTestAccess::set_state(k0, exact_t3_state(), false, false);
  expect(!sim::EngineTestAccess::complete(k0),
         "The Turo-Vessel route used exact hidden composition before K1.");
}

void test_pineco_seed_35_keeps_existing_t2() {
  const SeedResult result =
      run_seed("regidrago-pineco", "strict-jit/go-second", 35);
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Pineco seed 35 lost its existing T2 route.");
}

void test_shell_seed_43_keeps_existing_t2() {
  const SeedResult result =
      run_seed("regidrago-shell", "strict-jit/go-first", 43);
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Shell seed 43 lost its existing T2 route.");
}
}  // namespace

int main() {
  test_seed_26_plays_turo_before_vessel();
  test_both_k1_provenances_and_k0_boundary();
  test_pineco_seed_35_keeps_existing_t2();
  test_shell_seed_43_keeps_existing_t2();
  return 0;
}
