#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

bool contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_17_attaches_one_grass_then_promotes() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1561 fixture is unavailable.");

  std::mt19937_64 rng{17};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound K1 state has a current-turn Dragon payload, a Benched GGF
  // Regidrago VSTAR, Active Forretress ex, and searchable Grass. Exploding Energy
  // must choose at least one Grass because it is an Ability, attach that card, then
  // apply its printed self-Knock-Out so Regidrago VSTAR can be promoted on T3:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Matchup-flex timing and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1561
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 17 missed its legal T3 ready state.");
  expect(contains(trace, "Exploding Energy attached 1 Basic Grass Energy"),
         "Seed 17 did not make the required nonzero Ability selection.");
  expect(contains(trace, "T3 | PROMOTE") && contains(trace, "T3 | READY"),
         "Seed 17 did not self-Knock Out and promote Regidrago VSTAR on T3.");
}

}  // namespace

int main() {
  try {
    test_seed_17_attaches_one_grass_then_promotes();
    std::cout << "Issue 1561 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
