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
void test_seed_104_uses_t1_vessel_and_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1552 fixture is unavailable.");
  std::mt19937_64 rng{104};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  // Earthen Vessel can spend route-replaced Mysterious Treasure on T1, establish
  // K1, and preserve Quick Ball for the T2 Tapu Lele-GX to Crispin continuation:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1552
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 104 did not reach strict-JIT readiness on turn two.");
  expect(trace_contains(trace, "Mysterious Treasure (Earthen Vessel issue-1552 route cost)") &&
             trace_contains(trace, "Quick Ball issue-1552 route cost") &&
             trace_contains(trace, "T2 | WONDER TAG") &&
             trace_contains(trace, "T2 | READY |"),
         "Seed 104 did not execute the source-bound Vessel to Quick Ball route.");
}
}
int main() {
  try {
    test_seed_104_uses_t1_vessel_and_reaches_t2();
    std::cout << "Issue 1552 T1 Vessel route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
