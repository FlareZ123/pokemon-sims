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
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void expect_seed_346_t4(const char* scenario_label) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2225 fixture is unavailable.");

  std::mt19937_64 rng(346);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T2 Arven already established K1. On T4 the Active VSTAR is GF, the manual
  // attachment is unused, Arven can search Earthen Vessel, and Vessel can discard
  // held Dragapult ex while searching Grass. This consumes the same Supporter
  // permission as Gladion and completes Energy plus same-turn payload together.
  // The merged #2227 Treasure selector is on a distinct Item-cost path, so this
  // fixture also serves as a current-main non-interaction check for that merge.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Gladion / Oricorio comparison: https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sm2-55
  // Official Supporter, Item, discard, search, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, supporter contention, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/2225 https://github.com/FlareZ123/pokemon-sims/issues/2227
  expect(outcome.first_ready_turn == 4,
         "Seed 346 did not reach readiness on T4.");
  expect(trace_contains(trace,
                        "Arven searched Earthen Vessel for the complete Energy-plus-payload route"),
         "Seed 346 did not choose the Arven-Vessel Supporter route.");
  expect(trace_contains(trace, "Dragapult ex (Earthen Vessel cost)"),
         "Seed 346 did not use Dragapult ex as Vessel's payload cost.");
  expect(!trace_contains(trace, "exchanged Gladion for Oricorio"),
         "Seed 346 still spent the T4 Supporter on Gladion-Oricorio.");
}

void test_strict_and_matchup_flex_seed_346() {
  expect_seed_346_t4("strict-jit/go-first");
  expect_seed_346_t4("matchup-flex-jit/go-first");
}
}  // namespace

int main() {
  try {
    test_strict_and_matchup_flex_seed_346();
    std::cout << "Issue 2225 Arven-Vessel supporter-route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}