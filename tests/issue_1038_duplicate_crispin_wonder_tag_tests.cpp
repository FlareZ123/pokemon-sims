#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

int main() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing matchup-flex-jit/go-first scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{364};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  engine.run();

  const bool held_on_t2 = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T2 | HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Crispin") != std::string::npos;
      });
  const bool duplicate_t2_crispin = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T2 | WONDER TAG") != std::string::npos &&
               line.find("Crispin") != std::string::npos;
      });
  const bool played_held_crispin = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-CRISPIN-01") != std::string::npos;
      });

  // Wonder Tag searches one Supporter. A held Gladion already consumes the T2
  // Supporter action for the known Prize exchange, while the held Crispin remains
  // available for T3. The second T2 Wonder Tag therefore adds no legal route. A
  // later Wonder Tag for a different live Supporter remains permitted:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1038
  if (!held_on_t2 || duplicate_t2_crispin || !played_held_crispin) {
    throw std::runtime_error("Seed 364 still spent the T2 Wonder Tag for duplicate Crispin");
  }
  return 0;
}
