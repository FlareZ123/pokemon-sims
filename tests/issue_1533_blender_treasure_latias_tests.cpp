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

void test_seed_1_uses_treasure_latias_before_blender() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1533 fixture is unavailable.");

  std::mt19937_64 rng{1};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Mysterious Treasure may spend the surplus Fire, search Latias ex, and Bench
  // it before Brilliant Blender supplies the current-turn payload. Skyliner then
  // promotes the already-GGF VSTAR on the earliest complete turn:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 1 did not reach strict-JIT readiness on turn four.");
  expect(trace_contains(trace, "Fire Energy (Mysterious Treasure issue-1533 Latias route cost)") &&
             trace_contains(trace, "T4 | PLAY ITEM | rules: R-MT-01") &&
             trace_contains(trace, "T4 | BENCH | rules: R-GAME-BENCH | Latias ex") &&
             trace_contains(trace, "T4 | PLAY ITEM | rules: R-BLENDER-01") &&
             trace_contains(trace, "T4 | RETREAT | rules: R-LATIAS-01") &&
             trace_contains(trace, "T4 | READY"),
         "Seed 1 did not execute the Treasure-Latias-Blender completion route.");
}
}

int main() {
  try {
    test_seed_1_uses_treasure_latias_before_blender();
    std::cout << "Issue 1533 Treasure-Latias-Blender tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
