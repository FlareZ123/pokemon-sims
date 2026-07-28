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
  const NamedDeck* deck = deck_by_id("regidrago-pineco");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1673 registered setup is unavailable.");
  }

  std::mt19937_64 rng(183);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Secret Box requires three other cards discarded from hand. That mandatory
  // cost can establish the current-turn Dragon payload after Regidrago VSTAR has
  // already completed its GGF Energy axis:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1673
  if (outcome.first_ready_turn != 5 ||
      !trace_contains(trace, "Secret Box discarded the held Dragon payload") ||
      !trace_contains(trace, "Dragapult ex (Secret Box cost)") ||
      !trace_contains(trace, "T5 | READY")) {
    throw std::runtime_error(
        "Seed 183 did not complete the playable Secret Box payload route on T5.");
  }
}
