#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.late_steven_vstar_with_held_blender_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn = 2) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, turn - 1, 2, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV};
  return state;
}

bool available_with_state(const sim::DciProfile dci,
                          const sim::LockMode locks,
                          sim::State state,
                          const bool k1 = true,
                          const bool going_first = true,
                          const int max_turn = 5) {
  std::mt19937_64 rng(3225);
  const sim::Scenario scenario{"issue-3225", dci, locks, going_first, max_turn};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), k1);
  return sim::EngineTestAccess::route_available(engine);
}

bool available(const sim::DciProfile dci, const sim::LockMode locks,
               const int turn = 2, const int max_turn = 5) {
  return available_with_state(dci, locks, route_state(turn), true, true, max_turn);
}

void test_same_ready_turn_profiles_and_rule_box_lock() {
  // Steven's Resolve ends the current legal Supporter turn after reserving the
  // known Regidrago VSTAR. The already-held Brilliant Blender is played after the
  // prior-turn Regidrago V evolves on the following ready turn, putting a permitted
  // Dragon payload into discard on that same turn for either same-ready-turn JIT profile.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Supporter, Item, evolution, search, discard, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Rule Box Ability lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3225
  expect(available(sim::DciProfile::StrictJit, sim::LockMode::None),
         "StrictJit K1 held-Blender route should remain admitted.");
  expect(available(sim::DciProfile::MatchupFlexJit, sim::LockMode::None),
         "MatchupFlexJit should share the same-ready-turn held-Blender route.");
  expect(available(sim::DciProfile::MatchupFlexJit,
                   sim::LockMode::FullRuleBoxAbility),
         "Rule Box Ability lock should not suppress the Trainer-only route.");
}

void test_action_specific_locks_and_profile_boundaries() {
  // Steven's Resolve is governed by current Supporter legality, while Brilliant
  // Blender is governed by Item legality on the projected next turn. The repository
  // TurnTwoItem contract begins on turn two and persists afterward, so a projected
  // T3 Blender is blocked. NoDiscardControl is outside this helper's JIT route policy.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Persistent projected Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3225
  expect(!available(sim::DciProfile::NoDiscardControl, sim::LockMode::None),
         "NoDiscardControl should not enter the same-ready-turn held-Blender helper.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter),
         "Full Supporter lock should block Steven's Resolve.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::TurnTwoItem),
         "Persistent TurnTwoItem lock should block projected next-turn Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullItem),
         "Full Item lock should block projected next-turn Blender.");
  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::FullCombined),
         "Combined lock should block projected next-turn Blender.");
}

void test_k1_evolution_energy_and_horizon_requirements() {
  // This helper is the known-composition K1 route. It also requires a prior-turn
  // Regidrago V already paying Apex Dragon's GGF cost and one future turn in the
  // configured horizon before Steven can bank the searched VSTAR.
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Earliest-route/horizon policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed provenance and admission bugs: https://github.com/FlareZ123/pokemon-sims/issues/2085 https://github.com/FlareZ123/pokemon-sims/issues/3225
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               route_state(), false),
         "K0 state should not use the K1 held-Blender helper.");

  sim::State fresh = route_state();
  fresh.active->entered_turn = fresh.turn;
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               fresh),
         "A Regidrago V played this turn cannot use the projected evolution route.");

  sim::State incomplete_grass = route_state();
  incomplete_grass.active->grass = 1;
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               incomplete_grass),
         "Incomplete Grass energy should block the already-GGF route.");

  sim::State incomplete_fire = route_state();
  incomplete_fire.active->fire = 0;
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               incomplete_fire),
         "Incomplete Fire energy should block the already-GGF route.");

  expect(!available(sim::DciProfile::StrictJit, sim::LockMode::None, 5, 5),
         "Route should require a projected next turn inside the simulation horizon.");
}

void test_required_cards_and_payload_axes() {
  // The physical continuation requires held Steven and Blender, an exact known
  // Regidrago VSTAR in deck, a still-missing VSTAR axis, and at least one permitted
  // Dragon payload remaining in deck for Blender to discard on the ready turn.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3225
  sim::State missing_steven = route_state();
  missing_steven.hand.erase(std::remove(missing_steven.hand.begin(),
                                        missing_steven.hand.end(),
                                        sim::Card::StevensResolve),
                            missing_steven.hand.end());
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               missing_steven),
         "Missing Steven should block the route.");

  sim::State missing_blender = route_state();
  missing_blender.hand.erase(std::remove(missing_blender.hand.begin(),
                                         missing_blender.hand.end(),
                                         sim::Card::BrilliantBlender),
                             missing_blender.hand.end());
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               missing_blender),
         "Missing held Blender should block the route.");

  sim::State missing_vstar = route_state();
  missing_vstar.deck.erase(std::remove(missing_vstar.deck.begin(),
                                       missing_vstar.deck.end(),
                                       sim::Card::RegidragoVstar),
                           missing_vstar.deck.end());
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               missing_vstar),
         "Missing known VSTAR target should block the route.");

  sim::State missing_payload = route_state();
  missing_payload.deck.erase(std::remove(missing_payload.deck.begin(),
                                         missing_payload.deck.end(),
                                         sim::Card::Dragapult),
                             missing_payload.deck.end());
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               missing_payload),
         "Missing Blender payload should block the route.");

  sim::State vstar_already_held = route_state();
  vstar_already_held.hand.push_back(sim::Card::RegidragoVstar);
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               vstar_already_held),
         "Already-held VSTAR should leave this Steven-search route.");

  // Strict-JIT readiness is provenance-sensitive: being in discard is insufficient
  // unless the payload entered discard this turn. Mark both public discard presence
  // and current-turn provenance to exercise a truly completed payload axis.
  // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  sim::State payload_already_ready = route_state();
  payload_already_ready.discard.push_back(sim::Card::Dragapult);
  payload_already_ready.discarded_this_turn.push_back(sim::Card::Dragapult);
  expect(!available_with_state(sim::DciProfile::StrictJit, sim::LockMode::None,
                               payload_already_ready),
         "Already-ready payload axis should leave this missing-payload route.");
}
}  // namespace

int main() {
  try {
    test_same_ready_turn_profiles_and_rule_box_lock();
    test_action_specific_locks_and_profile_boundaries();
    test_k1_evolution_energy_and_horizon_requirements();
    test_required_cards_and_payload_axes();
    std::cout << "Issue 3225 held-Blender semantic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
