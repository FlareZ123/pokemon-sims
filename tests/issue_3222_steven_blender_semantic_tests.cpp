#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_3222_steven_blender_fallback_available();
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
  state.hand = {sim::Card::Grass, sim::Card::StevensResolve};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::Fire};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode locks,
               const bool going_first, const int turn,
               const int max_turn = 5, const bool deck_seen = true,
               const bool prizes_revealed = false) {
  std::mt19937_64 rng(3222);
  sim::Engine engine(sim::Scenario{"issue-3222", dci, locks, going_first, max_turn},
                     sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn), deck_seen,
                                   prizes_revealed);
  return sim::EngineTestAccess::route_available(engine);
}

void test_equivalent_turns_seats_and_jit_profiles() {
  // Steven banks VSTAR + Blender; the held Basic Energy completes GGF next turn,
  // and Blender supplies the Dragon payload on that same ready turn.
  // Steven: https://api.pokemontcg.io/v2/cards/sm7-145
  // Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3222
  for (const int turn : {2, 3, 4}) {
    expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, true, turn),
           "Strict JIT first-seat route retained a historical coordinate.");
    expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None, false, turn),
           "Matchup-flex second-seat route retained a historical coordinate.");
  }
}

void test_k1_provenance_and_k0_boundary() {
  // Repository K1 may come from either a legal deck inspection or a complete Prize
  // inspection. This fixed-list route must accept both while keeping true K0 out.
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Hisuian Heavy Ball / full Prize reveal: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3222
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None, false, 3,
                   5, false, true),
         "Prize-inspection K1 was rejected by the semantic Steven-Blender route.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, false, 3,
                    5, false, false),
         "True K0 entered the K1 Steven-Blender route.");
}

void test_action_specific_lock_semantics() {
  // Rule Box Ability suppression cannot stop Trainer cards. TurnTwoItem begins on
  // the player's second turn and remains active, so projected T3+ Blender stays
  // illegal after a T2 Steven play.
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Persistent Item-lock contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3326
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility,
                   false, 3),
         "Rule Box Ability lock incorrectly blocked Trainer-only Steven route.");
  expect(!available(sim::DciProfile::MatchupFlexJit, sim::LockMode::TurnTwoItem,
                    true, 2),
         "Persistent T2 Item lock admitted projected T3 Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem, true, 3),
         "Persistent T2 Item lock admitted projected T4 Blender.");

  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem, true, 3),
         "Full Item lock admitted projected Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullCombined, true, 3),
         "Combined lock admitted projected Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter, true, 3),
         "Supporter lock admitted Steven.");
}

void test_profile_and_horizon_boundaries() {
  // No-discard-control is outside this same-ready-turn JIT helper, and Steven needs
  // a following turn in which the completing attachment/evolution/Blender can occur.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/3222
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None, true, 3),
         "NoDiscardControl entered the same-ready-turn JIT helper.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, true, 5, 5),
         "Route ignored the missing future turn at the horizon.");
}
}  // namespace

int main() {
  try {
    test_equivalent_turns_seats_and_jit_profiles();
    test_k1_provenance_and_k0_boundary();
    test_action_specific_lock_semantics();
    test_profile_and_horizon_boundaries();
    std::cout << "Issue 3222 Steven-Blender semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
