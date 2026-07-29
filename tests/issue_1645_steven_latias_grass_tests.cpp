#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

}  // namespace

int main() {
  using namespace sim;

  const auto scenario = scenario_by_label("matchup-flex-jit/go-second");
  const CrobatModelingDeck* deck =
      crobat_modeling_deck_by_id("crobat1-heavy-ball");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1645 registered setup is unavailable.");
  }

  std::mt19937_64 rng(218);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Earthen Vessel resolves and attaches before Steven ends turn one. Steven then
  // searches Latias ex and one Grass, preserving held VSTAR and Blender. T2 evolves
  // and retreats the prepared Regidrago, while T3 Fire plus Blender reaches readiness:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, attachment, evolution, Supporter, retreat, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original refined route: https://github.com/FlareZ123/pokemon-sims/issues/1645
  // Confirmed pre-Steven ordering bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  if (outcome.first_ready_turn != 3 ||
      !trace_contains(trace,
                      "Earthen Vessel searched Grass and Fire before Steven's Resolve") ||
      !trace_contains(trace,
                      "Searched the complete post-Vessel T3 route: Latias ex, Grass Energy") ||
      !trace_contains(trace, "T2 | EVOLVE") ||
      !trace_contains(trace, "T2 | RETREAT") ||
      !trace_contains(trace, "T3 | READY")) {
    throw std::runtime_error(
        "Seed 218 did not complete the corrected pre-Steven Vessel route on T3.");
  }

  if (trace_contains(
          trace,
          "Searched up to 3 cards: Regidrago V, Regidrago VSTAR, Gladion") ||
      trace_contains(trace,
                     "Searched the complete Latias-Grass T4 route")) {
    throw std::runtime_error(
        "Seed 218 still selected a slower Steven target package.");
  }
}
