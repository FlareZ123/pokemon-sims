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

sim::TrialOutcome run_seed(const std::string& scenario_label,
                           sim::TraceLog& trace) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id("crobat2-erika-channeler");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1705 fixture is unavailable.");
  std::mt19937_64 rng{461};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return engine.run();
}

void test_seed_461_uses_complete_t2_route() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome = run_seed("strict-jit/go-second", trace);

  // Quick Ball establishes K1 before either strict-JIT discard. Field Blower is
  // route-replaced only in the lock-free state, and Brilliant Blender replaces
  // the early Dragapult payload on the T2 ready turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1705
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 461 did not reach strict-JIT readiness on T2.");
  expect(trace_contains(trace, "Quick Ball issue-1705 route") &&
             trace_contains(trace, "searched Latias ex") &&
             trace_contains(trace, "Blender-replaced Dragapult ex") &&
             trace_contains(trace, "complete T2 route") &&
             trace_contains(trace, "T2 | READY"),
         "Seed 461 did not execute the K1 Treasure-Latias-Vessel route.");
}

void test_rule_box_lock_preserves_route_costs() {
  sim::TraceLog trace{true, {}};
  (void)run_seed("strict-jit-rulebox-ability-lock/go-second", trace);
  expect(!trace_contains(trace, "issue-1705 route-replaced cost") &&
             !trace_contains(trace, "Blender-replaced Dragapult ex"),
         "The issue-1705 override spent protected costs through Rule Box Ability lock.");
}
}  // namespace

int main() {
  try {
    test_seed_461_uses_complete_t2_route();
    test_rule_box_lock_preserves_route_costs();
    std::cout << "Issue 1705 Treasure-Latias-Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
