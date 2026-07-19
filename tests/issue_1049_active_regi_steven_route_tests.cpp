#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

// Re-run this exact regression after synchronizing the branch with current main.
int main() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing matchup-flex-jit/go-first scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{9};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  const int wonder_tags = static_cast<int>(std::count_if(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T1 | WONDER TAG") != std::string::npos;
      }));
  const bool selected_steven = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T1 | WONDER TAG") != std::string::npos &&
               line.find("Steven's Resolve") != std::string::npos;
      });
  const bool played_steven = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T2 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-STEVEN-01") != std::string::npos;
      });
  const bool ready_t3 = outcome.first_ready_turn == 3 &&
      std::any_of(trace.lines.begin(), trace.lines.end(),
          [](const std::string& line) {
            return line.find("T3 | READY") != std::string::npos;
          });

  // One Wonder Tag banks Steven. Earthen Vessel and Vital Dance provide
  // the manual attachments, while Steven reserves evolution and Blender:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1049
  if (wonder_tags != 1 || !selected_steven || !played_steven || !ready_t3) {
    throw std::runtime_error("Seed 9 still misses the deterministic Steven T3 route");
  }
  return 0;
}
