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
  std::mt19937_64 rng{120};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const auto has_trace = [&trace](const std::string& needle) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&needle](const std::string& line) {
                         return line.find(needle) != std::string::npos;
                       });
  };

  // Gladion must take the known prized Turo rather than the redundant Regidrago V.
  // Steven then reserves VSTAR and Grass, Quick Ball discards Dragapult ex on T4,
  // and Turo returns the attachment-free Dialga-GX to promote the ready attacker:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1030
  if (outcome.first_ready_turn != 4 ||
      !has_trace("exchanged Gladion for Professor Turo's Scenario") ||
      !has_trace("Searched the known T4 Turo promotion route: Regidrago VSTAR, Grass Energy") ||
      !has_trace("T4 | DISCARD | rules: R-QB-01 | Dragapult ex") ||
      !has_trace("Professor Turo returned the Basic Active Pokémon and promoted") ||
      has_trace("exchanged Gladion for Regidrago V.")) {
    throw std::runtime_error("Seed 120 did not execute the deterministic T4 Turo route");
  }
  return 0;
}
