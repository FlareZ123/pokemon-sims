#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_1955_plays_arven_searched_blender_on_turn_three() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1910 registered fixture is unavailable.");

  std::mt19937_64 rng{1955};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Arven may search Brilliant Blender, and the searched Item remains playable
  // during the same turn before the ready-state check:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, Item, search, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1910
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 1955 did not reach matchup-flex-JIT readiness on turn three.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-ARVEN-01") &&
             trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01") &&
             trace_contains(trace, "T3 | READY |"),
         "Seed 1955 did not replay the Arven-searched Blender on turn three.");
}
}

int main() {
  try {
    test_seed_1955_plays_arven_searched_blender_on_turn_three();
    std::cout << "Issue 1910 late Arven-Blender replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
