#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static bool route_available(const Engine& engine) {
    return engine.issue_1478_t1_field_blower_direct_regi_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::TrialOutcome run_seed_290(const sim::DciProfile dci, sim::TraceLog& trace) {
  const sim::Scenario scenario{"issue-2798", dci, sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{290};
  sim::Engine engine(scenario, recipe, rng, &trace);
  return engine.run();
}

void test_seed_290_same_turn_jit_parity() {
  // StrictJit and MatchupFlexJit share the same ready-turn payload rule, so the
  // public K0 Field Blower -> Mysterious Treasure route must be admitted by both:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2798
  sim::TraceLog strict_trace;
  strict_trace.enabled = true;
  const sim::TrialOutcome strict = run_seed_290(sim::DciProfile::StrictJit, strict_trace);
  expect(strict.first_ready_turn == 3,
         "StrictJit seed 290 must retain the proven T3 route.");

  sim::TraceLog flex_trace;
  flex_trace.enabled = true;
  const sim::TrialOutcome flex = run_seed_290(sim::DciProfile::MatchupFlexJit, flex_trace);
  expect(flex.first_ready_turn == 3,
         "MatchupFlexJit seed 290 must use the same legal T3 route.");
  expect(trace_contains(flex_trace,
                        "Discarded setup-dead Field Blower and searched Regidrago V"),
         "MatchupFlexJit must execute the public Field Blower direct-Regidrago route.");
}
}  // namespace

int main() {
  try {
    test_seed_290_same_turn_jit_parity();
    std::cout << "Issue 2798 MatchupFlex #1478 parity tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
