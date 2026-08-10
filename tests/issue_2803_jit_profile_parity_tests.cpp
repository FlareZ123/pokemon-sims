#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool t1_connector(const Engine& engine) {
    return engine.issue_1235_t1_quick_ball_connector_state();
  }
  static bool t2_completion(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_crispin_completion_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::DciProfile profile) {
  return sim::Scenario{"issue-2803", profile, sim::LockMode::None, false, 5};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::RegidragoV, sim::Card::DialgaGX,
                sim::Card::MysteriousTreasure, sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Fire, sim::Card::Arven};
  return state;
}

sim::State t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                 sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Gladion};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::Arven};
  return state;
}

void test_t1_and_t2_use_shared_jit_timing() {
  std::mt19937_64 rng{2803};
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  // Engine stores Scenario by const reference, so fixtures must keep each Scenario
  // alive for the Engine lifetime. Passing scenario(...) directly would leave a
  // dangling reference after the constructor full-expression:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // C++ temporary lifetime: https://eel.is/c++draft/class.temporary
  // Follow-up systemic bug: https://github.com/FlareZ123/pokemon-sims/issues/2815
  const sim::Scenario strict_scenario = scenario(sim::DciProfile::StrictJit);
  const sim::Scenario flex_scenario = scenario(sim::DciProfile::MatchupFlexJit);
  const sim::Scenario control_scenario = scenario(sim::DciProfile::NoDiscardControl);
  sim::Engine strict(strict_scenario, recipe, rng);
  sim::Engine flex(flex_scenario, recipe, rng);
  sim::Engine control(control_scenario, recipe, rng);

  sim::EngineTestAccess::set_state(strict, t1_state());
  sim::EngineTestAccess::set_state(flex, t1_state());
  sim::EngineTestAccess::set_state(control, t1_state());

  // Strict JIT and matchup-flex JIT share the same ready-turn payload timing;
  // no-discard-control permits earlier payload banking and stays outside this route.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2803
  expect(sim::EngineTestAccess::t1_connector(strict),
         "Strict JIT lost the #1235 T1 connector.");
  expect(sim::EngineTestAccess::t1_connector(flex),
         "Matchup-flex JIT still rejects the #1235 T1 connector.");
  expect(!sim::EngineTestAccess::t1_connector(control),
         "No-discard-control incorrectly entered the JIT-specific T1 connector.");

  sim::EngineTestAccess::set_state(strict, t2_state(), true);
  sim::EngineTestAccess::set_state(flex, t2_state(), true);
  sim::EngineTestAccess::set_state(control, t2_state(), true);
  expect(sim::EngineTestAccess::t2_completion(strict),
         "Strict JIT lost the #1235 T2 completion.");
  expect(sim::EngineTestAccess::t2_completion(flex),
         "Matchup-flex JIT still rejects the #1235 T2 completion.");
  expect(!sim::EngineTestAccess::t2_completion(control),
         "No-discard-control incorrectly entered the JIT-specific T2 completion.");
}

void test_seed_211_preserves_t2_readiness_for_both_jit_profiles() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict_scenario = scenario(sim::DciProfile::StrictJit);
  const sim::Scenario flex_scenario = scenario(sim::DciProfile::MatchupFlexJit);
  std::mt19937_64 strict_rng{211};
  std::mt19937_64 flex_rng{211};
  sim::Engine strict(strict_scenario, recipe, strict_rng);
  sim::Engine flex(flex_scenario, recipe, flex_rng);
  const sim::TrialOutcome strict_outcome = strict.run();
  const sim::TrialOutcome flex_outcome = flex.run();

  // Seed 211 is the fixed end-to-end reproduction. The exact-state assertions above
  // prove access to the #1235 route itself; this seed assertion protects the reported
  // observable regression boundary without overfitting to one equally-fast T2 trace.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Core procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed reproduction: https://github.com/FlareZ123/pokemon-sims/issues/2803
  // Independent CI re-verification of both T2 outcomes: https://github.com/FlareZ123/pokemon-sims/issues/2816
  expect(strict_outcome.first_ready_turn == 2,
         "Strict JIT seed 211 must remain ready on T2.");
  expect(flex_outcome.first_ready_turn == 2,
         "Matchup-flex JIT seed 211 must remain ready on T2.");
}
}  // namespace

int main() {
  try {
    test_t1_and_t2_use_shared_jit_timing();
    test_seed_211_preserves_t2_readiness_for_both_jit_profiles();
    std::cout << "Issue 2803 JIT profile parity tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
