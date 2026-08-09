#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_seed_855_treasure_spends_payload_before_latias_promotion() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2204 fixture unavailable");

  std::mt19937_64 rng{855};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Mysterious Treasure pays its discard before searching a Dragon or Psychic
  // Pokemon. Dialga-GX is the current-turn Apex Dragon payload, while Latias ex's
  // Skyliner removes the Basic Active's Retreat Cost so the already powered Benched
  // Regidrago VSTAR can become Active on the same turn.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Double Dragon Energy readiness handling: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Official Item, discard, search, Bench, Ability, Retreat, and promotion procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI, current-turn JIT, Active-position, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed stale-claim bug and abandoned prior PR: https://github.com/FlareZ123/pokemon-sims/issues/2204 https://github.com/FlareZ123/pokemon-sims/pull/2217
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2204 seed 855 did not reach T4 readiness");
  expect(has(trace, "Dialga-GX (Mysterious Treasure cost)"),
         "issue-2204 did not spend the held Dragon as Treasure's JIT payload cost");
  expect(has(trace, "Latias ex") && has(trace, "T4 | RETREAT") &&
             has(trace, "T4 | READY"),
         "issue-2204 did not complete the Latias promotion route on T4");
}
}  // namespace

int main() {
  try {
    test_seed_855_treasure_spends_payload_before_latias_promotion();
    std::cout << "Issue 2204 Treasure Latias payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
