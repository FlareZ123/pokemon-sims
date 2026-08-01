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

void test_seed_329_replays_blender_after_vstar_promotion() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1911 registered fixture is unavailable.");

  std::mt19937_64 rng{329};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Skyliner permits the transition into the evolved GGF Regidrago VSTAR without
  // ending the turn. The held Item remains playable before readiness is recorded.
  // The production gate uses prizes_known(), whose deck-search, Prize-inspection,
  // and true-K0 boundaries are covered by the repository public-K1 contract tests:
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official evolution, Ability, retreat, Item, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_1896_public_k1_treasure_tests.cpp
  // K1, current-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1911
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 329 did not reach matchup-flex-JIT readiness on turn four.");
  expect(trace_contains(trace, "T4 | PLAY ITEM | rules: R-BLENDER-01") &&
             trace_contains(trace, "T4 | READY |"),
         "Seed 329 did not replay held Blender after the VSTAR transition.");
}
}  // namespace

int main() {
  try {
    test_seed_329_replays_blender_after_vstar_promotion();
    std::cout << "Issue 1911 post-transition Blender replay tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
