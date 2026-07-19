#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

int main() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario");

  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{130};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const auto has_trace = [&trace](const std::string& needle) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&needle](const std::string& line) {
                         return line.find(needle) != std::string::npos;
                       });
  };

  // The T2 manual Fire attachment completes GGF. Mysterious Treasure may then
  // discard Dragapult ex, search Regidrago VSTAR, and permit the legal prior-turn
  // Regidrago V evolution before the unused Supporter slot is considered:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1032
  if (outcome.first_ready_turn != 2 ||
      !has_trace("T2 | DISCARD | rules: R-MT-01 | Dragapult ex") ||
      !has_trace("T2 | EVOLVE | rules: R-GAME-EVOLVE") ||
      has_trace("T2 | PLAY SUPPORTER | rules: R-BURNET-01")) {
    throw std::runtime_error(
        "Seed 130 did not reopen the same-turn Treasure evolution before Burnet");
  }
  return 0;
}
