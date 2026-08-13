#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.late_steven_has_active_regidrago_t3_blender_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 2, 0};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Fire, sim::Card::StevensResolve};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::Grass,
                sim::Card::RegidragoV};
  return state;
}

bool available(const sim::DciProfile dci, const sim::LockMode locks,
               const bool going_first, const int turn,
               const int max_turn = 5) {
  std::mt19937_64 rng(3228);
  const sim::Scenario scenario{"issue-3228", dci, locks, going_first, max_turn};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn));
  return sim::EngineTestAccess::route_available(engine);
}

void test_semantic_admission_across_turns_seats_and_jit_profiles() {
  // Steven's Resolve has no absolute-turn or seat restriction beyond normal
  // Supporter timing. Brilliant Blender supplies the Dragon payload on the next
  // ready turn, which is the same timing contract for both JIT profiles.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Supporter, Item, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-ready-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3228
  for (const int turn : {2, 3, 4}) {
    expect(available(sim::DciProfile::StrictJit, sim::LockMode::None,
                     true, turn),
           "StrictJit first-seat route retained a historical turn gate.");
    expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                     false, turn),
           "MatchupFlexJit second-seat route retained a profile/seat gate.");
  }
}

void test_rule_box_and_persistent_turn_two_item_lock_semantics() {
  // Rule Box Ability lock does not suppress these Trainer cards. TurnTwoItem is
  // persistent from the player's T2 onward, so every projected T3+ Blender play is
  // illegal even when Steven itself remains legal on the preceding turn.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Advanced Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L404
  // Persistent TurnTwoItem model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Shared projected-lock helper: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Confirmed persistent-lock bug: https://github.com/FlareZ123/pokemon-sims/issues/3403
  expect(available(sim::DciProfile::MatchupFlexJit,
                   sim::LockMode::FullRuleBoxAbility, false, 3),
         "Rule Box Ability lock incorrectly blocked the Trainer-only route.");
  expect(!available(sim::DciProfile::StrictJit,
                    sim::LockMode::TurnTwoItem, true, 2),
         "Persistent TurnTwoItem incorrectly admitted next-turn T3 Blender.");
  expect(!available(sim::DciProfile::MatchupFlexJit,
                    sim::LockMode::TurnTwoItem, false, 3),
         "Persistent TurnTwoItem incorrectly admitted a later Blender projection.");
}

void test_real_route_boundaries_remain_blocking() {
  // Full Item/combined locks suppress next-turn Brilliant Blender. Full Supporter
  // lock suppresses Steven. NoDiscardControl is outside this same-ready-turn JIT
  // helper, and the route needs one future turn inside the configured horizon.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3228
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                    true, 3),
         "Full Item lock incorrectly admitted next-turn Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullCombined,
                    true, 3),
         "Combined lock incorrectly admitted next-turn Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                    true, 3),
         "Supporter lock incorrectly admitted Steven's Resolve.");
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                    true, 3),
         "NoDiscardControl incorrectly entered the same-ready-turn JIT helper.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None,
                    true, 5, 5),
         "Route ignored the missing next turn beyond the simulation horizon.");
}
}  // namespace

int main() {
  try {
    test_semantic_admission_across_turns_seats_and_jit_profiles();
    test_rule_box_and_persistent_turn_two_item_lock_semantics();
    test_real_route_boundaries_remain_blocking();
    std::cout << "Issue 3228/3403 late-Steven semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
