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

  const bool held_existing_crispin = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T2 | HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Crispin") != std::string::npos;
      });
  const bool played_gladion_t2 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T2 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-GLADION-01") != std::string::npos;
      });
  const bool played_crispin_t2 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T2 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-CRISPIN-01") != std::string::npos;
      });
  const bool played_held_crispin_t3 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T3 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-CRISPIN-01") != std::string::npos;
      });

  // #3170 may generalize the #1795 Crispin -> Steven -> Vessel packet only when
  // that packet is not dominated by an equal-or-earlier observable route. In this
  // MatchupFlex witness, the held Gladion consumes T2 while the already-held Crispin
  // remains live for T3, so forcing #1795 to start with T2 Crispin would regress the
  // completed #1038 connector-preservation contract.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Connector/route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Preserved completed route: https://github.com/FlareZ123/pokemon-sims/issues/1038
  // Refined semantic-admission issue: https://github.com/FlareZ123/pokemon-sims/issues/3170
  if (!held_existing_crispin || !played_gladion_t2 || played_crispin_t2 ||
      !played_held_crispin_t3) {
    throw std::runtime_error(
        "#3170 refinement lost the connector-dominated MatchupFlex seed-364 route");
  }
  return 0;
}
