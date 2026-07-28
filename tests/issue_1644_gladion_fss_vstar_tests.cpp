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

  const auto scenario = scenario_by_label("no-discard-control/go-first");
  const CrobatModelingDeck* deck =
      crobat_modeling_deck_by_id("crobat2-erika-channeler");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1644 registered setup is unavailable.");
  }

  std::mt19937_64 rng(83);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Gladion may exchange itself for one known Prize, Forest Seal Stone grants
  // Star Alchemy to a Pokémon V, and Regidrago VSTAR may evolve from the
  // prior-turn Regidrago V after Star Alchemy searches it:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1644
  // Issue #1697 refines this route: a held Fire Energy makes T3 readiness
  // deterministic, so Gladion, Forest Seal Stone, Star Alchemy, and evolution
  // move to T2 while the original earliest T3 ready turn remains unchanged:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1697
  if (outcome.first_ready_turn != 3 ||
      !trace_contains(trace, "T2 | PLAY SUPPORTER") ||
      !trace_contains(trace, "Exchanged Gladion for Forest Seal Stone") ||
      !trace_contains(trace, "T2 | STAR ALCHEMY") ||
      !trace_contains(trace, "T2 | EVOLVE") ||
      !trace_contains(trace, "T3 | READY")) {
    throw std::runtime_error(
        "Seed 83 did not take the refined T2 Gladion-Forest Seal Stone-VSTAR route.");
  }

  if (trace_contains(trace, "Completed mandatory Prize exchange with Grass Energy")) {
    throw std::runtime_error("Seed 83 still selected inert Grass Energy from Prizes.");
  }
}
