#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_3221_k0_steven_blender_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Dragapult, sim::Card::Fire};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode locks,
               const bool going_first, const int turn,
               const int max_turn = 5, const bool prizes_revealed = false) {
  std::mt19937_64 rng(3221);
  sim::Engine engine(sim::Scenario{"issue-3221", dci, locks, going_first, max_turn},
                     sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn), prizes_revealed);
  return sim::EngineTestAccess::route_available(engine);
}

void test_k0_route_generalizes_across_equivalent_states() {
  // Steven performs the first real deck inspection, reserves VSTAR + Grass, and
  // held Blender supplies the same-ready-turn payload after the next-turn evolution.
  // Steven: https://api.pokemontcg.io/v2/cards/sm7-145
  // Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K0/K1: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3221
  for (const int turn : {2, 3, 4}) {
    expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, true, turn),
           "Strict JIT first-seat K0 route retained a historical turn gate.");
    expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, false, turn),
           "Matchup-flex second-seat K0 route retained a profile/seat gate.");
  }
}

void test_k0_rejects_prize_inspection_k1() {
  // Full Prize inspection is repository K1, even when no deck search has occurred.
  // The K0 helper must leave that known-composition state for the K1 route.
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Hisuian Heavy Ball / full Prize reveal: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3221
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, false, 3,
                    5, true),
         "Prize-inspection K1 incorrectly entered the K0 Steven-Blender route.");
}

void test_action_specific_lock_projection() {
  // Rule Box Ability lock does not suppress Steven or Blender. A T2-only Item lock
  // is expired before a T3 Blender following T2 Steven.
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3221
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility,
                   false, 3),
         "Rule Box Ability lock incorrectly blocked K0 Trainer route.");
  expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::TurnTwoItem,
                   true, 2),
         "Expired T2 Item lock incorrectly blocked T3 Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem, true, 3),
         "Full Item lock admitted K0 Blender route.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullCombined, true, 3),
         "Combined lock admitted K0 Blender route.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter, true, 3),
         "Supporter lock admitted K0 Steven route.");
}

void test_profile_and_horizon_boundaries() {
  // This helper models same-ready-turn JIT. It also needs one following turn for
  // the searched attachment/evolution plus held Blender completion.
  // JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3221
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None, true, 3),
         "NoDiscardControl entered the K0 same-ready-turn JIT route.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, true, 5, 5),
         "K0 route ignored horizon exhaustion.");
}
}  // namespace

int main() {
  try {
    test_k0_route_generalizes_across_equivalent_states();
    test_k0_rejects_prize_inspection_k1();
    test_action_specific_lock_projection();
    test_profile_and_horizon_boundaries();
    std::cout << "Issue 3221 K0 Steven-Blender semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
