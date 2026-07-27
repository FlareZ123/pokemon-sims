#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
void expect(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}
void exact_seed() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-1564 fixture unavailable");
  std::mt19937_64 rng{5};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1564
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "seed 5 did not reach no-control readiness on T3");
  expect(has(trace, "T3 | DISCARD") &&
             has(trace, "Quick Ball issue-1564 Latias route cost") &&
             has(trace, "T3 | BENCH") && has(trace, "Latias ex") &&
             has(trace, "T3 | RETREAT") && has(trace, "T3 | READY"),
         "seed 5 omitted the Quick Ball-Latias promotion");
}
}
int main() { exact_seed(); }
