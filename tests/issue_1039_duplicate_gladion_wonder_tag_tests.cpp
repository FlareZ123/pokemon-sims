#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

int main() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-first scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{172};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool held_on_t3 = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Gladion") != std::string::npos;
      });
  const bool duplicate_t3_gladion = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | WONDER TAG") != std::string::npos &&
               line.find("Gladion") != std::string::npos;
      });
  const bool used_held_gladion = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-GLADION-01") != std::string::npos &&
               line.find("Earthen Vessel") != std::string::npos;
      });

  // Wonder Tag would search another Gladion for the same known Prize axis. The held
  // copy already performs that mandatory exchange, and only one Supporter can be
  // played this turn. Holding Tapu preserves the Bench slot without changing T3:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1039
  if (outcome.first_ready_turn != 3 || !held_on_t3 ||
      duplicate_t3_gladion || !used_held_gladion) {
    throw std::runtime_error("Seed 172 still spent Wonder Tag for duplicate held Gladion");
  }
  return 0;
}
