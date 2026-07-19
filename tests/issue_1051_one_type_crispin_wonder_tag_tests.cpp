\
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

int main() {
  const auto scenario = sim::scenario_by_label("strict-jit-turn2-item-lock/go-second");
  if (!scenario) {
    throw std::runtime_error("Missing strict-jit-turn2-item-lock/go-second scenario");
  }
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{57};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  engine.run();

  const bool held_on_t4 = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T4 | HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Crispin") != std::string::npos;
      });
  const bool duplicate_t4_crispin = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T4 | WONDER TAG") != std::string::npos &&
               line.find("Crispin") != std::string::npos;
      });
  const bool played_held_crispin = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T5 | PLAY SUPPORTER") != std::string::npos &&
               line.find("Only one Basic Energy type was searchable") != std::string::npos;
      });
  const bool manual_grass = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T5 | ATTACH") != std::string::npos &&
               line.find("Grass Energy manually to Regidrago VSTAR") != std::string::npos;
      });

  // Crispin may search up to two Basic Energy of different types. When only
  // one needed type remains, that card goes to hand and the still-unused
  // manual attachment advances GGF. A second Wonder Tag for the same held
  // Supporter therefore has no marginal route:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1051
  if (!held_on_t4 || duplicate_t4_crispin || !played_held_crispin ||
      !manual_grass) {
    throw std::runtime_error(
        "Seed 57 still spent Wonder Tag for a duplicate one-type Crispin route");
  }
  return 0;
}
