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
      sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1702 fixture is unavailable.");
  std::mt19937_64 rng{300};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return engine.run();
}

void test_seed_300_uses_uniquely_prized_latias() {
  sim::TraceLog trace{true, {}};
  const sim::TrialOutcome outcome = run_seed("strict-jit/go-second", trace);

  // Heavy Ball's Prize inspection establishes K1. The route preserves the uniquely
  // stranded Latias axis, uses payable Treasure for Regidrago, and spends Channeler
  // only after every T2 route component is publicly proven:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Channeler: https://api.pokemontcg.io/v2/cards/sm11-190
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1702
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 300 did not reach strict-JIT readiness on T2.");
  expect(trace_contains(trace, "uniquely prized Latias ex") &&
             trace_contains(trace, "route-replaced Channeler") &&
             trace_contains(trace, "Searched any card: Regidrago VSTAR") &&
             trace_contains(trace, "complete T2 route") &&
             trace_contains(trace, "T2 | READY"),
         "Seed 300 did not execute the K1 Heavy-Ball-Latias route.");
}

void test_rule_box_lock_preserves_channeler() {
  sim::TraceLog trace{true, {}};
  (void)run_seed("strict-jit-rulebox-ability-lock/go-second", trace);
  expect(!trace_contains(trace, "route-replaced Channeler") &&
             !trace_contains(trace, "uniquely prized Latias ex"),
         "The issue-1702 override crossed the Rule Box Ability-lock gate.");
}
}  // namespace

int main() {
  try {
    test_seed_300_uses_uniquely_prized_latias();
    test_rule_box_lock_preserves_channeler();
    std::cout << "Issue 1702 Heavy-Ball-Latias tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
