#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
  }

  static bool initial_visible(const Engine& engine) {
    return engine.issue_1796_t2_steven_route_available();
  }

  static bool wonder_tag_selector_visible(const Engine& engine) {
    return engine.issue_1796_wonder_tag_steven_route_available();
  }

  static bool finish(Engine& engine) {
    engine.issue_1796_finish_turn_ = engine.state_.turn;
    return engine.complete_issue_1796_t3_finish();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State initial_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Gladion, sim::Card::TapuLeleGX};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  state.manual_energy_used = true;
  return state;
}

sim::State selector_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::TapuLeleGX, turn, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grass,
                sim::Card::Gladion};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite,
                sim::Card::QuickBall};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State finish_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::Oricorio, turn - 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::RegidragoV};
  state.prizes = {sim::Card::FieldBlower, sim::Card::Serena,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool visible(const sim::DciProfile dci, const sim::LockMode lock,
             const bool going_first, sim::State state, const int max_turn,
             const bool known = true) {
  const sim::Scenario scenario{"issue-3173", dci, lock, going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3173);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::initial_visible(engine);
}

bool selector_visible(const sim::DciProfile dci, const sim::LockMode lock,
                      const bool going_first, sim::State state,
                      const int max_turn, const bool known = true) {
  const sim::Scenario scenario{"issue-3173-selector", dci, lock,
                               going_first, max_turn};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3176);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::wonder_tag_selector_visible(engine);
}

void test_initial_semantic_admission() {
  // Mysterious Treasure supplies the current-turn Dragon discard while Latias ex
  // supplies the Basic Active promotion axis. StrictJit and MatchupFlexJit share
  // the same ready-turn payload timing; seat and absolute turn add no card rule:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  expect(visible(sim::DciProfile::MatchupFlexJit, sim::LockMode::None,
                 true, initial_state(2), 3),
         "MatchupFlexJit hid the legal issue-1796 route");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 false, initial_state(2), 3),
         "Equivalent going-second state was suppressed");
  expect(visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                 true, initial_state(3), 4),
         "Equivalent later-turn state was suppressed");
}

void test_initial_semantic_boundaries() {
  // Latias ex is a Rule Box Pokémon whose Ability is required by this route, while
  // Mysterious Treasure is an Item and Steven's Resolve is a Supporter:
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Scenario lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  expect(!visible(sim::DciProfile::StrictJit,
                  sim::LockMode::FullRuleBoxAbility, true,
                  initial_state(2), 3),
         "Rule Box Ability lock admitted the Skyliner-dependent route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullItem,
                  true, initial_state(2), 3),
         "Item lock admitted the Treasure-dependent route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::FullSupporter,
                  true, initial_state(2), 3),
         "Supporter lock admitted the Steven-Crispin route");
  expect(!visible(sim::DciProfile::NoDiscardControl, sim::LockMode::None,
                  true, initial_state(2), 3),
         "NoDiscardControl entered the same-turn-JIT route");
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, initial_state(2), 3, false),
         "K0 exposed a K1 deterministic route");

  sim::State same_turn_regi = initial_state(2);
  same_turn_regi.bench.front().entered_turn = 2;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(same_turn_regi), 3),
         "Same-turn Regidrago V satisfied evolution timing");

  sim::State retreat_spent = initial_state(2);
  retreat_spent.retreat_used = true;
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(retreat_spent), 3),
         "Spent Retreat admitted the Skyliner route");

  sim::State full_bench = initial_state(2);
  while (full_bench.bench.size() < 5U) {
    full_bench.bench.push_back(
        sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  }
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(full_bench), 3),
         "Full Bench admitted the Latias ex route");

  sim::State no_treasure = initial_state(2);
  no_treasure.hand.erase(
      std::remove(no_treasure.hand.begin(), no_treasure.hand.end(),
                  sim::Card::MysteriousTreasure),
      no_treasure.hand.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_treasure), 3),
         "Missing Mysterious Treasure admitted the route");

  sim::State no_latias = initial_state(2);
  no_latias.deck.erase(
      std::remove(no_latias.deck.begin(), no_latias.deck.end(),
                  sim::Card::LatiasEx),
      no_latias.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_latias), 3),
         "Missing Latias ex admitted the route");

  sim::State no_vstar = initial_state(2);
  no_vstar.deck.erase(
      std::remove(no_vstar.deck.begin(), no_vstar.deck.end(),
                  sim::Card::RegidragoVstar),
      no_vstar.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_vstar), 3),
         "Missing Regidrago VSTAR admitted the route");

  sim::State no_crispin = initial_state(2);
  no_crispin.deck.erase(
      std::remove(no_crispin.deck.begin(), no_crispin.deck.end(),
                  sim::Card::Crispin),
      no_crispin.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_crispin), 3),
         "Missing Crispin admitted the route");

  sim::State no_payload = initial_state(2);
  no_payload.deck.erase(
      std::remove(no_payload.deck.begin(), no_payload.deck.end(),
                  sim::Card::MegaDragonite),
      no_payload.deck.end());
  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, std::move(no_payload), 3),
         "Missing Dragon payload admitted the route");

  expect(!visible(sim::DciProfile::StrictJit, sim::LockMode::None,
                  true, initial_state(2), 2),
         "Route ignored the next-turn horizon");
}

void test_wonder_tag_selector_semantics() {
  // Wonder Tag must choose the same banked Steven line under either same-turn JIT
  // profile, either seat when the current Supporter is legal, and equivalent later
  // turns. The held Mysterious Treasure is used on the following turn, so a
  // scheduled turn-two Item lock must block a T1-going-second bank even though
  // Items are still legal on the current turn:
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Turn-two Item-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Same-turn JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Advanced Supporter/Item/evolution/Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed remaining selector bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  expect(selector_visible(sim::DciProfile::MatchupFlexJit,
                          sim::LockMode::None, true,
                          selector_state(2), 3),
         "MatchupFlexJit hid the Wonder Tag Steven selector");
  expect(selector_visible(sim::DciProfile::StrictJit,
                          sim::LockMode::None, false,
                          selector_state(2), 3),
         "Going-second T2 hid the Wonder Tag Steven selector");
  expect(selector_visible(sim::DciProfile::StrictJit,
                          sim::LockMode::None, true,
                          selector_state(3), 4),
         "Equivalent later turn hid the Wonder Tag Steven selector");
  expect(selector_visible(sim::DciProfile::StrictJit,
                          sim::LockMode::None, false,
                          selector_state(1), 2),
         "Legal going-second T1 bank was suppressed");

  expect(!selector_visible(sim::DciProfile::NoDiscardControl,
                           sim::LockMode::None, false,
                           selector_state(1), 2),
         "NoDiscardControl entered the same-turn-JIT selector");
  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::FullRuleBoxAbility, true,
                           selector_state(2), 3),
         "Rule Box Ability lock admitted the Wonder Tag/Skyliner route");
  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::TurnTwoItem, false,
                           selector_state(1), 2),
         "Scheduled T2 Item lock failed to block next-turn Treasure");
  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::FullSupporter, true,
                           selector_state(2), 3),
         "Supporter lock admitted the banked Steven route");
  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true,
                           selector_state(2), 3, false),
         "K0 exposed the K1 Wonder Tag selector");

  sim::State same_turn_regi = selector_state(2);
  same_turn_regi.bench.front().entered_turn = 2;
  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true,
                           std::move(same_turn_regi), 3),
         "Same-turn Regidrago V admitted the next-turn evolution route");

  expect(!selector_visible(sim::DciProfile::StrictJit,
                           sim::LockMode::None, true,
                           selector_state(2), 2),
         "Wonder Tag selector ignored its next-turn horizon");
}

void test_finish_semantics() {
  // The banked next-turn finish must retain shared JIT, seat-independent, and
  // later-turn semantics while preserving the real Treasure, Supporter, Latias,
  // Bench, Retreat, and Ability constraints:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Same-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3173
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario scenario{"issue-3173-finish",
      sim::DciProfile::MatchupFlexJit, sim::LockMode::None, false, 5};
  std::mt19937_64 rng(3174);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, finish_state(4));
  expect(sim::EngineTestAccess::finish(engine),
         "Equivalent later-turn MatchupFlexJit finish was suppressed");

  const sim::Scenario ability_lock{"issue-3173-ability-lock",
      sim::DciProfile::StrictJit, sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 lock_rng(3175);
  sim::Engine locked_engine(ability_lock, recipe, lock_rng);
  sim::EngineTestAccess::set_state(locked_engine, finish_state(4));
  expect(!sim::EngineTestAccess::finish(locked_engine),
         "Rule Box Ability lock allowed the Latias-dependent finish");
}

}  // namespace

int main() {
  test_initial_semantic_admission();
  test_initial_semantic_boundaries();
  test_wonder_tag_selector_semantics();
  test_finish_semantics();
  return 0;
}
