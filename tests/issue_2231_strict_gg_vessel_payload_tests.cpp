#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool discarded(const sim::Engine& engine, const sim::Card card) {
  const auto& cards = sim::EngineTestAccess::state(engine).discard;
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect_seed_ready_with_vessel_payload(const char* scenario_label,
                                           const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2231 fixture is unavailable.");

  std::mt19937_64 rng(seed);
  sim::Engine engine(*scenario, deck->recipe, rng);
  const sim::TrialOutcome outcome = engine.run();

  // At public K1 with Active Regidrago VSTAR at GG, Earthen Vessel can search
  // Fire and the unused manual attachment completes GGF. A held Dragapult ex is
  // therefore DCI 1 for this exact Vessel cost because that payment also creates
  // the required current-turn Apex Dragon payload. This exact GG witness remains
  // distinct from merged #2224, which covers the opposite GF Energy orientation.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bugs defining the two orientations: https://github.com/FlareZ123/pokemon-sims/issues/2224 https://github.com/FlareZ123/pokemon-sims/issues/2231
  expect(outcome.first_ready_turn == 4,
         "Issue 2231 seed did not reach readiness on T4.");
  expect(discarded(engine, sim::Card::Dragapult),
         "Issue 2231 seed did not leave Dragapult ex in the discard pile.");
  expect(!discarded(engine, sim::Card::Dipplin),
         "Issue 2231 seed discarded Dipplin instead of the payload.");
}

void test_strict_seed_witnesses() {
  expect_seed_ready_with_vessel_payload("strict-jit/go-first", 5059);
  expect_seed_ready_with_vessel_payload("strict-jit/go-first", 5161);
}

void test_matchup_flex_controls() {
  expect_seed_ready_with_vessel_payload("matchup-flex-jit/go-first", 5059);
  expect_seed_ready_with_vessel_payload("matchup-flex-jit/go-first", 5161);
}
}  // namespace

int main() {
  try {
    test_strict_seed_witnesses();
    test_matchup_flex_controls();
    std::cout << "Issue 2231 strict GG Vessel payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}