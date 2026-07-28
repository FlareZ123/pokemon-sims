#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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

  const auto scenario = scenario_by_label("matchup-flex-jit/go-first");
  const NamedDeck* deck = deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1672 registered setup is unavailable.");
  }

  std::mt19937_64 rng(12);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Star Alchemy searches any card. Earthen Vessel discards one card and searches
  // Basic Energy. Latias ex gives the Basic Active no Retreat Cost, and Apex Dragon
  // requires GGF plus a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1672
  if (outcome.first_ready_turn != 5 ||
      !trace_contains(trace, "Searched Latias ex while Earthen Vessel retained") ||
      !trace_contains(trace, "T5 | DISCARD") ||
      !trace_contains(trace, "Dialga-GX") ||
      !trace_contains(trace, "T5 | BENCH") ||
      !trace_contains(trace, "Latias ex from hand") ||
      !trace_contains(trace, "T5 | RETREAT") ||
      !trace_contains(trace, "T5 | READY")) {
    throw std::runtime_error(
        "Seed 12 did not complete the Forest Seal Stone, Vessel, and Latias route on T5.");
  }

  if (trace_contains(trace, "Searched any card: Brilliant Blender")) {
    throw std::runtime_error(
        "Seed 12 still spent Star Alchemy on Brilliant Blender instead of Latias ex.");
  }
}
