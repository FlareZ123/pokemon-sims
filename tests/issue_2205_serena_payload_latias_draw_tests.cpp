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
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}
}
int main() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || !deck) throw std::runtime_error("issue-2205 fixture unavailable");
  std::mt19937_64 rng{1134};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2205
  if (outcome.first_ready_turn != 4 || outcome.setup_failed ||
      !has(trace, "Serena issue-2205 optional payload discard") ||
      !has(trace, "Dragapult ex") || !has(trace, "Mysterious Treasure") ||
      !has(trace, "Latias ex") || !has(trace, "T4 | RETREAT") ||
      !has(trace, "T4 | READY")) {
    throw std::runtime_error("issue-2205 route regression");
  }
  return 0;
}
