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

void expect_seed_ready_with_treasure_payload(const char* scenario_label) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2230 fixture is unavailable.");

  std::mt19937_64 rng(5484);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The prior-turn Regidrago V already has GG. Mysterious Treasure can discard
  // Mega Dragonite ex, search Regidrago VSTAR, and held Crispin can attach Fire
  // before evolution. That single Treasure cost creates the required current-turn
  // Dragon payload while the same continuation completes GGF and the VSTAR axis.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Supporter, Energy, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2230
  expect(outcome.first_ready_turn == 4,
         "Seed 5484 did not reach readiness on T4.");
  expect(trace_contains(trace, "Mega Dragonite ex (Mysterious Treasure cost)"),
         "Seed 5484 did not spend the held Dragon as Treasure's cost.");
  expect(!trace_contains(trace, "Grass Energy (Mysterious Treasure cost)"),
         "Seed 5484 still spent Grass Energy instead of the payload.");
}

void test_strict_and_matchup_flex_seed_5484() {
  expect_seed_ready_with_treasure_payload("strict-jit/go-second");
  expect_seed_ready_with_treasure_payload("matchup-flex-jit/go-second");
}
}  // namespace

int main() {
  try {
    test_strict_and_matchup_flex_seed_5484();
    std::cout << "Issue 2230 Treasure Crispin VSTAR payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}