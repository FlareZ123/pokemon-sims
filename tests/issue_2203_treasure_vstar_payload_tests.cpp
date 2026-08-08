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
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || !deck) throw std::runtime_error("issue-2203 fixture unavailable");

  std::mt19937_64 rng{1376};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1/DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2203
  if (outcome.first_ready_turn != 2 || outcome.setup_failed ||
      !has(trace, "Dialga-GX (Mysterious Treasure cost)") ||
      !has(trace, "Regidrago VSTAR") || !has(trace, "T2 | EVOLVE") ||
      !has(trace, "T2 | READY")) {
    throw std::runtime_error("issue-2203 route regression");
  }
  return 0;
}
