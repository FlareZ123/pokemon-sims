#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_6_replays_arven_searched_blender_on_turn_three() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-klara");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1565 fixture is unavailable.");

  std::mt19937_64 rng{6};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  std::cerr << "PR2489 issue-1565 diagnostic trace\n";
  for (const auto& line : trace.lines) std::cerr << line << '\n';

  // Legacy Star may recover Arven, Arven may search Brilliant Blender, and the
  // searched Item may be played later in that same turn before the ready check:
  // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core Supporter and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1565
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 6 did not reach strict-JIT readiness on turn three.");
  expect(trace_contains(trace, "T3 | LEGACY STAR |") &&
             trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-ARVEN-01") &&
             trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01") &&
             trace_contains(trace, "T3 | READY |"),
         "Seed 6 did not replay the Arven-searched Blender on turn three.");
}
}

int main() {
  try {
    test_seed_6_replays_arven_searched_blender_on_turn_three();
    std::cout << "Issue 1565 Legacy Star Arven-Blender replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
