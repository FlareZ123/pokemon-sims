#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains_issue_1552_vessel_route(const sim::DciProfile dci) {
  const sim::Scenario scenario{
      "issue-3039/exact", dci, sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(165);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  engine.run();

  for (const std::string& line : trace.lines) {
    if (line.find("Earthen Vessel issue-1552 T1 route") != std::string::npos) {
      return true;
    }
  }
  return false;
}

void test_same_turn_jit_profiles_share_t1_vessel_staging() {
  // Seed 165 is the established T1 Earthen Vessel witness. StrictJit and
  // MatchupFlexJit use the same ready-turn Dragon-payload timing, so the same
  // physical K0 staging route must be available to both profiles:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route / confirmed generalization: https://github.com/FlareZ123/pokemon-sims/issues/1552 https://github.com/FlareZ123/pokemon-sims/issues/3039
  expect(trace_contains_issue_1552_vessel_route(sim::DciProfile::StrictJit),
         "StrictJit lost the established issue-1552 T1 Vessel staging route.");
  expect(trace_contains_issue_1552_vessel_route(sim::DciProfile::MatchupFlexJit),
         "MatchupFlexJit remained blocked by the historical profile identity.");
}

void test_no_discard_control_remains_outside_jit_specific_route() {
  // NoDiscardControl has no same-ready-turn payload restriction, so it must stay
  // outside this JIT-specific staging fallback even if generic Vessel play is legal:
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed scope: https://github.com/FlareZ123/pokemon-sims/issues/3039
  expect(!trace_contains_issue_1552_vessel_route(sim::DciProfile::NoDiscardControl),
         "The JIT-specific issue-1552 T1 Vessel fallback leaked into NoDiscardControl.");
}

}  // namespace

int main() {
  try {
    test_same_turn_jit_profiles_share_t1_vessel_staging();
    test_no_discard_control_remains_outside_jit_specific_route();
    std::cout << "Issue 3039 shared-JIT T1 Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
