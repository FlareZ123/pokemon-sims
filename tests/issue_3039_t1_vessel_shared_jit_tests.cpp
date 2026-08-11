#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

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

bool profile_uses_ready_turn_payload_timing(const sim::DciProfile dci) {
  const sim::Scenario scenario{
      "issue-3039/profile", dci, sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3039);
  sim::Engine engine(scenario, recipe, rng);
  return sim::EngineTestAccess::uses_ready_turn_payload_timing(engine);
}

void test_shared_jit_semantic_predicate() {
  // The production #1552 T1 Vessel gate uses this semantic predicate. Both JIT
  // profiles require a Dragon payload to enter discard on the ready turn, while
  // NoDiscardControl does not. The CI witness traces separately verify the real
  // seed-165 Vessel action under both registered JIT scenarios.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route / confirmed generalization: https://github.com/FlareZ123/pokemon-sims/issues/1552 https://github.com/FlareZ123/pokemon-sims/issues/3039
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
    test_shared_jit_semantic_predicate();
    std::cout << "Issue 3039 shared-JIT semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
