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

  // Issue #1552 supersedes the older T3 Vessel hold for this exact seed. A
  // public T1 Earthen Vessel search establishes K1 and pays route-replaced
  // Mysterious Treasure. On T2, Quick Ball discards Dialga-GX, searches Tapu
  // Lele-GX, Wonder Tag finds Crispin, and Crispin plus the manual Fire complete
  // GGF. The Dragon enters discard on the same ready turn, so strict JIT is met:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original timing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1447
  // Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1552
  expect(outcome.first_ready_turn == 2,
         "Strict-JIT seed 104 lost its earliest T2 ready turn.");
  expect(trace_contains(trace,
                        "T1 | DISCARD | rules: R-EV-01; P-DCI-01; P-COMPRESS-01 | Mysterious Treasure") &&
             trace_contains(trace,
                            "T2 | DISCARD | rules: R-QB-01; P-DCI-01; P-JIT-01 | Dialga-GX") &&
             trace_contains(trace, "T2 | WONDER TAG") &&
             trace_contains(trace, "Crispin") &&
             trace_contains(trace, "T2 | READY"),
         "The source-bound trace did not execute the T1 Vessel to T2 Quick Ball route.");
  expect(!trace_contains(trace, "R-BLENDER-01"),
         "The faster route consumed Brilliant Blender.");
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
