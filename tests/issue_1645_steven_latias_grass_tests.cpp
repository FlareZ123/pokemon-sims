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

  // Steven searches up to three cards, Latias ex gives Basic Pokémon no Retreat
  // Cost, Earthen Vessel searches Basic Energy after one discard, Brilliant Blender
  // establishes the current-turn payload, and Apex Dragon requires GGF:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1645
  if (outcome.first_ready_turn != 4 ||
      !trace_contains(trace,
                      "Searched the complete Latias-Grass T4 route: Latias ex, Grass Energy") ||
      !trace_contains(trace, "T2 | EVOLVE") ||
      !trace_contains(trace, "T2 | RETREAT") ||
      !trace_contains(trace, "T4 | READY")) {
    throw std::runtime_error(
        "Seed 218 did not complete the direct Latias-Grass route on T4.");
  }

  if (trace_contains(
          trace,
          "Searched up to 3 cards: Regidrago V, Regidrago VSTAR, Gladion")) {
    throw std::runtime_error(
        "Seed 218 still selected the redundant Regidrago V-VSTAR-Gladion package.");
  }
}
