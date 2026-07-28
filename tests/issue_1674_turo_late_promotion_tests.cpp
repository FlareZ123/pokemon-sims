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

  const auto scenario = scenario_by_label("strict-jit/go-first");
  const NamedDeck* deck = deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1674 registered setup is unavailable.");
  }

  std::mt19937_64 rng(897);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Professor Turo's Scenario returns the Basic Active Pokémon to hand. The
  // completed Benched Regidrago VSTAR then becomes Active after Earthen Vessel has
  // established the same-turn Dragon payload and the manual attachment completes GGF:
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1674
  if (outcome.first_ready_turn != 5 ||
      !trace_contains(trace, "Professor Turo returned the Basic Active Pokémon") ||
      !trace_contains(trace, "Completed the deadline Professor Turo promotion route") ||
      !trace_contains(trace, "T5 | READY")) {
    throw std::runtime_error(
        "Seed 897 did not use Professor Turo to promote the complete VSTAR on T5.");
  }
}
