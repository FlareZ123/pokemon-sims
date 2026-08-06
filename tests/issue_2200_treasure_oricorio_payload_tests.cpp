#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace {

bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

}  // namespace

int main() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || !deck) throw std::runtime_error("issue-2200 fixture unavailable");

  std::mt19937_64 rng{242};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Bench, Ability, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, current-turn JIT, DCI, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2200
  if (outcome.first_ready_turn != 3 || outcome.setup_failed ||
      !has(trace, "Mysterious Treasure issue-2200 current-turn payload cost") ||
      !has(trace, "Dragapult ex") || !has(trace, "searched Oricorio") ||
      !has(trace, "T3 | READY")) {
    throw std::runtime_error("issue-2200 seed omitted the Dragon-cost Oricorio finish");
  }
  return 0;
}
