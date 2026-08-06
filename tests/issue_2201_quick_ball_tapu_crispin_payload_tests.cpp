#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) { return line.find(needle) != std::string::npos; });
}
void check(const char* scenario_label, const std::uint64_t seed, const int ready_turn,
           const char* payload) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || !deck) throw std::runtime_error("issue-2201 fixture unavailable");
  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Hisuian Goodra VSTAR and Dragapult ex: https://api.pokemontcg.io/v2/cards/swsh11-136 https://api.pokemontcg.io/v2/cards/sv6-130
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2201
  if (outcome.first_ready_turn != ready_turn || outcome.setup_failed ||
      !has(trace, "Quick Ball issue-2201 Tapu-Crispin payload cost") ||
      !has(trace, payload) || !has(trace, "WONDER TAG") ||
      !has(trace, "T" + std::to_string(ready_turn) + " | READY")) {
    throw std::runtime_error("issue-2201 route regression");
  }
}
}
int main() {
  check("strict-jit/go-second", 1333, 3, "Hisuian Goodra VSTAR");
  check("matchup-flex-jit/go-first", 623, 4, "Dragapult ex");
  return 0;
}
