#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

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

sim::TrialOutcome run_seed_104(const std::string& scenario_label,
                               sim::TraceLog& trace) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1447 fixture is unavailable.");

  std::mt19937_64 rng(104);
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return engine.run();
}

void test_strict_jit_preserves_vessel_for_t3_payload() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome = run_seed_104("strict-jit/go-first", trace);

  // Crispin plus the T2 manual attachment creates GF. Earthen Vessel can then
  // discard Dialga-GX on T3, search the final Grass, and preserve the singleton
  // Brilliant Blender while retaining the same earliest ready turn:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1447
  expect(outcome.first_ready_turn == 3,
         "Strict-JIT seed 104 lost its earliest T3 ready turn.");
  expect(!trace_contains(trace,
                         "T2 | DISCARD | rules: R-EV-01 | Dialga-GX"),
         "Earthen Vessel still discarded Dialga-GX before the ready turn.");
  expect(trace_contains(trace,
                        "T3 | DISCARD | rules: R-EV-01 | Dialga-GX "
                        "(Earthen Vessel cost)"),
         "The preserved Vessel did not establish the T3 strict-JIT payload.");
  expect(!trace_contains(trace, "R-BLENDER-01"),
         "The same-deadline route still consumed Brilliant Blender.");
  expect(trace_contains(trace, "T3 | READY"),
         "The source-bound trace did not report T3 readiness.");
}

void test_no_discard_control_keeps_early_vessel() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome =
      run_seed_104("no-discard-control/go-first", trace);

  // No-discard-control permits early payload banking, so the strict-JIT hold must
  // not alter its existing Earthen Vessel route:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1447
  expect(outcome.first_ready_turn == 3,
         "No-discard-control seed 104 changed its ready turn.");
  expect(trace_contains(trace,
                        "T1 | DISCARD | rules: R-EV-01 | Dialga-GX "
                        "(Earthen Vessel cost)"),
         "The strict-JIT guard leaked into no-discard-control.");
}

void test_matchup_flex_keeps_existing_route() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome =
      run_seed_104("matchup-flex-jit/go-first", trace);

  // Matchup-flex has its own dynamic DCI route. The issue-1447 change is scoped
  // to the stricter resource proof and must leave this profile unchanged:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1447
  expect(outcome.first_ready_turn == 3,
         "Matchup-flex seed 104 changed its ready turn.");
  expect(trace_contains(trace,
                        "T2 | DISCARD | rules: R-EV-01 | Dialga-GX "
                        "(Earthen Vessel cost)"),
         "The strict-only Vessel hold changed matchup-flex DCI behavior.");
}

void test_turn_two_item_lock_does_not_delay_vessel() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome =
      run_seed_104("strict-jit-turn2-item-lock/go-first", trace);

  // A delayed T3 Vessel is unavailable once the modeled Item lock begins on T2.
  // The exact no-lock guard must preserve the existing legal lock route:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1447
  expect(outcome.first_ready_turn == 4,
         "Turn-two Item-lock seed 104 changed its legal ready turn.");
  expect(!trace_contains(trace, "Earthen Vessel cost"),
         "The lock control illegally played Earthen Vessel.");
}

}  // namespace

int main() {
  try {
    test_strict_jit_preserves_vessel_for_t3_payload();
    test_no_discard_control_keeps_early_vessel();
    test_matchup_flex_keeps_existing_route();
    test_turn_two_item_lock_does_not_delay_vessel();
    std::cout << "Issue 1447 Vessel timing tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
