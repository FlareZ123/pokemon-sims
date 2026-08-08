#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void expect_seed_ready(const char* scenario_label) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2229 fixture is unavailable.");

  std::mt19937_64 rng(1518);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The prior-turn Regidrago V is already GGF and evolution-legal. Mysterious
  // Treasure can spend Mega Dragonite ex as its printed cost, search the VSTAR,
  // and evolve on the same turn. The Supporter action is unused and irrelevant
  // because this complete route requires only the Item and evolution action.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2229
  expect(outcome.first_ready_turn == 4,
         "Seed 1518 did not reach readiness on T4.");
  expect(trace_contains(trace, "Mega Dragonite ex (Mysterious Treasure cost)"),
         "Seed 1518 did not spend the held Dragon as Treasure's cost.");
  expect(!trace_contains(trace, "Fire Energy (Mysterious Treasure cost)"),
         "Seed 1518 still spent Fire Energy instead of the payload.");
}

void test_strict_and_matchup_flex_seed_1518() {
  expect_seed_ready("strict-jit/go-first");
  expect_seed_ready("matchup-flex-jit/go-first");
}
}  // namespace

int main() {
  try {
    test_strict_and_matchup_flex_seed_1518();
    std::cout << "Issue 2229 unused-Supporter Treasure VSTAR tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
