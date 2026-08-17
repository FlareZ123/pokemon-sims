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
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool wonder_tag_route(const Engine& engine) {
    return engine.issue_4060_wonder_tag_bank_route_available_under_action_locks();
  }
  static bool banked_route(const Engine& engine) {
    return engine.issue_4060_banked_steven_route_available_under_action_locks();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State turn_one_state(const bool path_removed = false) {
  sim::State state;
  state.turn = 1;
  state.path_lock_removed = path_removed;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::Arven};
  return state;
}

sim::State turn_two_no_control_state(const bool path_removed = false) {
  sim::State state;
  state.turn = 2;
  state.path_lock_removed = path_removed;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Arven};
  state.discard = {sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  return state;
}

sim::State turn_two_strict_state(const bool path_removed = false) {
  sim::State state = turn_two_no_control_state(path_removed);
  state.discard.clear();
  state.hand.push_back(sim::Card::BrilliantBlender);
  state.deck.push_back(sim::Card::MegaDragonite);
  return state;
}

bool wonder_tag_available(const sim::DciProfile profile,
                          const sim::LockMode locks,
                          const bool path_removed = false) {
  const sim::Scenario scenario{"issue-4060-t1", profile, locks, true, 4};
  std::mt19937_64 rng{406001};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, turn_one_state(path_removed));
  return sim::EngineTestAccess::wonder_tag_route(engine);
}

bool banked_available(const sim::DciProfile profile,
                      const sim::LockMode locks,
                      const bool path_removed = false) {
  const sim::Scenario scenario{"issue-4060-t2", profile, locks, true, 4};
  std::mt19937_64 rng{406002};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, profile == sim::DciProfile::NoDiscardControl
                  ? turn_two_no_control_state(path_removed)
                  : turn_two_strict_state(path_removed));
  return sim::EngineTestAccess::banked_route(engine);
}

void test_no_discard_turn_two_item_uses_only_t1_blender_window() {
  // TurnTwoItem leaves Blender legal on T1. NoDiscardControl may complete the
  // payload axis then, so the banked T2 Steven continuation has no later Item
  // dependency and remains legal even though Items are locked by T2.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Turn-two Item timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Canonical Item-lock primitive: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // No-discard policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4060
  expect(wonder_tag_available(sim::DciProfile::NoDiscardControl,
                              sim::LockMode::TurnTwoItem),
         "T1 legal Blender window did not admit the no-discard Wonder Tag route.");
  expect(banked_available(sim::DciProfile::NoDiscardControl,
                          sim::LockMode::TurnTwoItem),
         "Banked no-discard payload invented a later Item dependency.");
}

void test_removed_path_restores_latias_route() {
  // Path-style suppression matters only while the Stadium remains active. Once
  // removed, Latias ex's Skyliner is available and the strict-JIT route may keep
  // Blender for the legal T3 Item window.
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Canonical Ability-lock primitive: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4060
  expect(!wonder_tag_available(sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false),
         "Live Path-style suppression admitted a Latias-dependent route.");
  expect(wonder_tag_available(sim::DciProfile::StrictJit,
                              sim::LockMode::FullRuleBoxAbility, true),
         "Removed Path-style suppression did not restore the Wonder Tag route.");
  expect(banked_available(sim::DciProfile::StrictJit,
                          sim::LockMode::FullRuleBoxAbility, true),
         "Removed Path-style suppression did not restore the banked Steven route.");
}

void test_combined_and_hard_lock_boundaries() {
  // FullCombined has a legal T1 Item window but also Path-style Ability
  // suppression. With that Stadium removed, NoDiscardControl may bank Blender on
  // T1 and continue through the T2 Item lock. FullItem blocks the T1 Blender action,
  // and FullSupporter blocks the future Steven action.
  // Combined lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#combined-lock
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4060
  expect(!wonder_tag_available(sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullCombined, false),
         "Combined lock admitted the route while Path-style suppression was live.");
  expect(wonder_tag_available(sim::DciProfile::NoDiscardControl,
                              sim::LockMode::FullCombined, true),
         "Combined lock with removed Path lost its legal T1 Blender route.");
  expect(banked_available(sim::DciProfile::NoDiscardControl,
                          sim::LockMode::FullCombined, true),
         "Combined lock rejected the already-banked no-discard payload on T2.");
  expect(!wonder_tag_available(sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullItem, true),
         "FullItem admitted the T1 Blender-dependent route.");
  expect(!wonder_tag_available(sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullSupporter, true),
         "FullSupporter admitted a route that requires Steven next turn.");
  expect(!banked_available(sim::DciProfile::StrictJit,
                           sim::LockMode::TurnTwoItem, true),
         "Strict JIT admitted a route whose T3 Blender is Item-locked.");
}
}  // namespace

int main() {
  try {
    test_no_discard_turn_two_item_uses_only_t1_blender_window();
    test_removed_path_restores_latias_route();
    test_combined_and_hard_lock_boundaries();
    std::cout << "Issue 4060 Steven action-lock tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
