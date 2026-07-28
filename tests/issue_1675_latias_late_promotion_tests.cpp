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

  const auto scenario = scenario_by_label("strict-jit/go-second");
  const NamedDeck* deck = deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1675 registered setup is unavailable.");
  }

  std::mt19937_64 rng(1907);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Mysterious Treasure discards Dialga-GX as this turn's Dragon payload. Latias ex
  // may then be Benched, and Skyliner gives the Basic Active no Retreat Cost so the
  // completed Benched Regidrago VSTAR can become Active:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1675
  if (outcome.first_ready_turn != 5 ||
      !trace_contains(trace, "Dialga-GX") ||
      !trace_contains(trace, "Latias ex") ||
      !trace_contains(trace, "Completed the deadline Latias ex promotion route") ||
      !trace_contains(trace, "T5 | READY")) {
    throw std::runtime_error(
        "Seed 1907 did not Bench Latias ex and promote the complete VSTAR on T5.");
  }
}
