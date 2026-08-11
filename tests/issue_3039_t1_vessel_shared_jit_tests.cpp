#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static bool uses_ready_turn_payload_timing(const Engine& engine) {
    return engine.strict_payload_timing();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::TraceLog trace_for_profile(const sim::DciProfile dci) {
  const sim::Scenario scenario{
      "issue-3039/exact", dci, sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(165);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  engine.run();
  return trace;
}

bool has_t1_vessel_staging(const sim::TraceLog& trace) {
  bool paid_vessel_with_treasure = false;
  bool searched_both_basic_energy = false;
  for (const std::string& line : trace.lines) {
    if (line.find("T1 | DISCARD | rules: R-EV-01 | Mysterious Treasure (Earthen Vessel cost)") !=
        std::string::npos) {
      paid_vessel_with_treasure = true;
    }
    if (line.find("T1 | Earthen Vessel | rules: R-EV-01; R-GAME-ITEM | Searched up to 2 Basic Energy: Grass Energy, Fire Energy.") !=
        std::string::npos) {
      searched_both_basic_energy = true;
    }
  }
  return paid_vessel_with_treasure && searched_both_basic_energy;
}

bool profile_uses_ready_turn_payload_timing(const sim::DciProfile dci) {
  const sim::Scenario scenario{
      "issue-3039/profile", dci, sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3039);
  sim::Engine engine(scenario, recipe, rng);
  return sim::EngineTestAccess::uses_ready_turn_payload_timing(engine);
}

void test_same_turn_jit_profiles_share_t1_vessel_staging() {
  // Seed 165 is the established T1 Earthen Vessel witness. StrictJit and
  // MatchupFlexJit use the same ready-turn Dragon-payload timing, so the same
  // physical K0 staging route must spend the redundant Treasure and search G+F:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route / confirmed generalization: https://github.com/FlareZ123/pokemon-sims/issues/1552 https://github.com/FlareZ123/pokemon-sims/issues/3039
  const sim::TraceLog strict = trace_for_profile(sim::DciProfile::StrictJit);
  const sim::TraceLog flex = trace_for_profile(sim::DciProfile::MatchupFlexJit);
  expect(has_t1_vessel_staging(strict),
         "StrictJit lost the established issue-1552 T1 Vessel staging state.");
  expect(has_t1_vessel_staging(flex),
         "MatchupFlexJit remained blocked by the historical profile identity.");
}

void test_non_jit_profile_remains_excluded_by_semantic_predicate() {
  // The production gate now uses this semantic predicate directly. Both JIT
  // profiles carry same-ready-turn payload timing; NoDiscardControl does not:
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/3039
  expect(profile_uses_ready_turn_payload_timing(sim::DciProfile::StrictJit),
         "StrictJit lost ready-turn payload timing.");
  expect(profile_uses_ready_turn_payload_timing(sim::DciProfile::MatchupFlexJit),
         "MatchupFlexJit lost ready-turn payload timing.");
  expect(!profile_uses_ready_turn_payload_timing(sim::DciProfile::NoDiscardControl),
         "NoDiscardControl unexpectedly entered the JIT-specific predicate.");
}

}  // namespace

int main() {
  try {
    test_same_turn_jit_profiles_share_t1_vessel_staging();
    test_non_jit_profile_remains_excluded_by_semantic_predicate();
    std::cout << "Issue 3039 shared-JIT T1 Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
