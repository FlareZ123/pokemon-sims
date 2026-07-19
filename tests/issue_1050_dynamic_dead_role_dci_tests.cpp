#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

// Re-run this exact regression after synchronizing the branch with current main.
int main() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-first scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{87};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  const bool vessel_cost = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T4 | DISCARD") != std::string::npos &&
               line.find("Quick Ball (Earthen Vessel cost)") != std::string::npos;
      });
  const bool arven_vessel = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T4 | PLAY SUPPORTER") != std::string::npos &&
               line.find("Arven") != std::string::npos &&
               line.find("Earthen Vessel") != std::string::npos;
      });
  const bool ready_t4 = outcome.first_ready_turn == 4 &&
      std::any_of(trace.lines.begin(), trace.lines.end(),
          [](const std::string& line) {
            return line.find("T4 | READY") != std::string::npos;
          });

  // After Active Regidrago VSTAR, payload, and GF are established, Quick
  // Ball no longer advances setup. Arven may find Earthen Vessel, which
  // discards that dead role, searches Grass, and enables the legal manual
  // attachment for the earliest known T4 ready state:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1050
  if (!vessel_cost || !arven_vessel || !ready_t4) {
    throw std::runtime_error("Seed 87 still leaves the final Arven-Vessel route blocked");
  }
  return 0;
}
