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

void test_seed_6_uses_cheaper_direct_vessel_finish_on_turn_three() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat1-klara");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1565 fixture is unavailable.");

  std::mt19937_64 rng{6};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Powerglass has already attached the final Grass at the end of T2, so T3
  // begins with an Apex-ready GGF attacker. The already-held Earthen Vessel then
  // has a cheaper equal-turn role than Legacy Star -> Arven -> Brilliant Blender:
  // its mandatory cost discards Mega Dragonite ex as the current-turn payload and
  // its legal Basic-Energy search resolves without spending the VSTAR Power,
  // Supporter action, or ACE SPEC.
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core Item, discard-cost, end-of-turn Tool, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Equal-turn resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Historical replay defect: https://github.com/FlareZ123/pokemon-sims/issues/1565
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 6 did not reach strict-JIT readiness on turn three.");
  expect(trace_contains(trace, "T2 | POWERGLASS | rules: R-POWERGLASS-01 | Attached Grass Energy from discard at end of turn.") &&
             trace_contains(trace, "T3 | POLICY | rules: P-AXIS-01 | Start: Active=Regidrago VSTAR [GGF] + Powerglass") &&
             trace_contains(trace, "T3 | DISCARD | rules: R-EV-01 | Mega Dragonite ex (Earthen Vessel cost)") &&
             trace_contains(trace, "T3 | Earthen Vessel | rules: R-EV-01; R-GAME-ITEM | Searched up to 2 Basic Energy: Grass Energy, Grass Energy.") &&
             trace_contains(trace, "T3 | READY |"),
         "Seed 6 did not execute the direct Powerglass-Vessel strict-JIT finish.");
  expect(!trace_contains(trace, "T3 | LEGACY STAR |") &&
             !trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-ARVEN-01") &&
             !trace_contains(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01"),
         "Seed 6 spent a constrained connector despite the direct Vessel finish.");
}
}

int main() {
  try {
    test_seed_6_uses_cheaper_direct_vessel_finish_on_turn_three();
    std::cout << "Issue 1565 direct Powerglass-Vessel completion tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
