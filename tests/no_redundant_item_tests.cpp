#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static void play_items_until_stable(Engine& engine, const bool permit_payload) {
    engine.play_items_until_stable(permit_payload);
  }
  static bool play_field_blower(Engine& engine) {
    return engine.play_field_blower();
  }
  static bool garbodor_locked(const Engine& engine) {
    return engine.garbodor_abilities_locked();
  }
  static bool pokemon_ability_available(const Engine& engine, const Card card) {
    return engine.ability_available_for_pokemon_garbodor(card);
  }
  static bool play_arven(Engine& engine) {
    return engine.play_arven();
  }
  static bool use_fss(Engine& engine) {
    return engine.use_fss();
  }
  static void begin_turn(Engine& engine, const int turn) {
    engine.begin_turn(turn);
  }
  static int garbodor_unlocked_turn(const Engine& engine) {
    return engine.garbodor_unlocked_turn_;
  }
};

}  // namespace sim

namespace {

sim::State minimal_garbodor_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite, sim::Card::Crispin};
  return state;
}

void test_ready_state_holds_hisuian_heavy_ball() {
  using namespace sim;
  const Scenario scenario{"ready-state-holds-heavy-ball", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(147);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::HisuianHeavyBall};
  state.prizes = {Card::RegidragoV, Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Heavy Ball's Prize reveal/exchange is optional, so a ready setup policy keeps
  // the Item instead of spending it without an unresolved axis: https://api.pokemontcg.io/v2/cards/swsh10-146
  EngineTestAccess::play_items_until_stable(engine, true);

  assert(std::count(state.hand.begin(), state.hand.end(), Card::HisuianHeavyBall) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::HisuianHeavyBall) == 0);
  assert(std::count(state.prizes.begin(), state.prizes.end(), Card::RegidragoV) == 1);
}

void test_ready_state_holds_field_blower() {
  using namespace sim;
  const Scenario scenario{"ready-state-holds-field-blower", DciProfile::StrictJit,
                          LockMode::FullRuleBoxAbility, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(791);
  Engine engine(scenario, recipe, rng);

  State state;
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::FieldBlower, Card::TapuLeleGX};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_state(engine, std::move(state));

  // Field Blower may discard Path to the Peak, but a completed setup has no live
  // Wonder Tag, Legacy Star, or Skyliner setup axis left to unlock. Preserve the
  // discrete-value Stadium answer instead of spending it automatically:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/791
  assert(!EngineTestAccess::play_field_blower(engine));
  const State& after = EngineTestAccess::state(engine);
  assert(std::count(after.hand.begin(), after.hand.end(), Card::FieldBlower) == 1);
  assert(std::count(after.discard.begin(), after.discard.end(), Card::FieldBlower) == 0);
  assert(!after.path_lock_removed);
}

void test_field_blower_restores_live_wonder_tag_route() {
  using namespace sim;
  const Scenario scenario{"field-blower-restores-wonder-tag", DciProfile::StrictJit,
                          LockMode::FullRuleBoxAbility, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(792);
  Engine engine(scenario, recipe, rng);

  State state;
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::FieldBlower, Card::TapuLeleGX};
  state.deck = {Card::ProfessorBurnet, Card::MegaDragonite};
  EngineTestAccess::set_state(engine, std::move(state));

  // Path suppresses Tapu Lele-GX's Wonder Tag. Field Blower is valuable here
  // because removing Path immediately restores a live Burnet payload connector:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  assert(EngineTestAccess::play_field_blower(engine));
  const State& after = EngineTestAccess::state(engine);
  assert(after.path_lock_removed);
  assert(std::count(after.hand.begin(), after.hand.end(), Card::FieldBlower) == 0);
  assert(std::count(after.discard.begin(), after.discard.end(), Card::FieldBlower) == 1);
}

void test_field_blower_restores_live_legacy_star_route() {
  using namespace sim;
  const Scenario scenario{"field-blower-restores-legacy-star", DciProfile::StrictJit,
                          LockMode::FullRuleBoxAbility, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(793);
  Engine engine(scenario, recipe, rng);

  State state;
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::FieldBlower};
  state.deck = {Card::MegaDragonite, Card::Grass, Card::Fire};
  EngineTestAccess::set_state(engine, std::move(state));

  // Path suppresses Regidrago VSTAR's Legacy Star. An unused VSTAR Power and an
  // unresolved payload axis keep Field Blower live even when Wonder Tag is absent:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sm2-125
  assert(EngineTestAccess::play_field_blower(engine));
  assert(EngineTestAccess::state(engine).path_lock_removed);
}

void test_garbodor_boost_shake_timing_by_seat() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();

  std::mt19937_64 first_rng(280801);
  const Scenario first{"garbodor-shake-ability-lock/go-first",
                       DciProfile::StrictJit, LockMode::None, true, 5};
  Engine first_engine(first, recipe, first_rng);
  State first_state = minimal_garbodor_state(1);
  EngineTestAccess::set_state(first_engine, first_state);

  // Going first, Regidrago T1 happens before the opponent's Boost Shake turn.
  // Garbotoxin begins on Regidrago T2. Going second, the opponent has already used
  // Boost Shake, so Garbotoxin is active on Regidrago T1:
  // https://api.pokemontcg.io/v2/cards/swsh7-142
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(!EngineTestAccess::garbodor_locked(first_engine));
  assert(EngineTestAccess::pokemon_ability_available(first_engine, Card::Oricorio));

  first_state.turn = 2;
  EngineTestAccess::set_state(first_engine, first_state);
  assert(EngineTestAccess::garbodor_locked(first_engine));
  assert(!EngineTestAccess::pokemon_ability_available(first_engine, Card::Oricorio));
  assert(!EngineTestAccess::pokemon_ability_available(first_engine, Card::CrobatV));

  std::mt19937_64 second_rng(280802);
  const Scenario second{"garbodor-shake-ability-lock/go-second",
                        DciProfile::StrictJit, LockMode::None, false, 5};
  Engine second_engine(second, recipe, second_rng);
  EngineTestAccess::set_state(second_engine, minimal_garbodor_state(1));
  assert(EngineTestAccess::garbodor_locked(second_engine));
}

void test_garbodor_field_blower_unlock_is_current_turn_only() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(280803);
  const Scenario scenario{"garbodor-shake-ability-lock/go-second",
                          DciProfile::StrictJit, LockMode::None, false, 5};
  Engine engine(scenario, recipe, rng);
  State state = minimal_garbodor_state(1);
  state.hand = {Card::FieldBlower, Card::CrobatV};
  EngineTestAccess::set_state(engine, std::move(state));

  // Field Blower can discard Garbodor's Tool, which ends Garbotoxin while that Tool
  // is absent. The scenario assumes maximum pressure and another Tool before the
  // next Regidrago turn, so the unlock is keyed only to the current turn:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(EngineTestAccess::play_field_blower(engine));
  assert(EngineTestAccess::garbodor_unlocked_turn(engine) == 1);
  assert(EngineTestAccess::pokemon_ability_available(engine, Card::CrobatV));

  EngineTestAccess::begin_turn(engine, 2);
  assert(EngineTestAccess::garbodor_locked(engine));
}

void test_garbodor_dark_asset_six_card_boundary_is_live() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(280804);
  const Scenario scenario{"garbodor-shake-ability-lock/go-second",
                          DciProfile::StrictJit, LockMode::None, false, 5};
  Engine engine(scenario, recipe, rng);
  State state = minimal_garbodor_state(1);
  state.hand = {Card::FieldBlower, Card::CrobatV, Card::Grass,
                Card::Fire, Card::RegidragoVstar, Card::Crispin};
  EngineTestAccess::set_state(engine, std::move(state));

  // Dark Asset checks hand size after Crobat V moves from hand to the Bench. A
  // six-card pre-play hand therefore becomes five and can draw one card:
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(EngineTestAccess::play_field_blower(engine));
}

void test_garbodor_only_actual_item_lock_blocks_field_blower() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();

  std::mt19937_64 ability_rng(280805);
  const Scenario ability_only{"garbodor-shake-ability-lock/go-second",
                              DciProfile::StrictJit, LockMode::None, false, 5};
  Engine ability_engine(ability_only, recipe, ability_rng);
  State ability_state = minimal_garbodor_state(1);
  ability_state.hand = {Card::FieldBlower, Card::CrobatV};
  EngineTestAccess::set_state(ability_engine, std::move(ability_state));
  assert(EngineTestAccess::play_field_blower(ability_engine));

  std::mt19937_64 item_rng(280806);
  const Scenario item_lock{"garbodor-shake-ability-lock/go-second",
                           DciProfile::StrictJit, LockMode::FullItem, false, 5};
  Engine item_engine(item_lock, recipe, item_rng);
  State item_state = minimal_garbodor_state(1);
  item_state.hand = {Card::FieldBlower, Card::CrobatV};
  EngineTestAccess::set_state(item_engine, std::move(item_state));

  // Garbotoxin suppresses Pokemon Abilities. Item legality remains governed by the
  // actual Item-lock rule:
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(!EngineTestAccess::play_field_blower(item_engine));
}

void test_garbodor_arven_searches_field_blower_for_live_unlock() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(280807);
  const Scenario scenario{"garbodor-shake-ability-lock/go-second",
                          DciProfile::StrictJit, LockMode::None, false, 5};
  Engine engine(scenario, recipe, rng);
  State state = minimal_garbodor_state(1);
  state.hand = {Card::Arven, Card::CrobatV};
  state.deck = {Card::FieldBlower, Card::ForestSealStone,
                Card::RegidragoVstar, Card::Grass, Card::Fire, Card::MegaDragonite};
  EngineTestAccess::set_state(engine, std::move(state));

  // Arven may search one Item. Here Field Blower immediately unlocks the held Dark
  // Asset connector, while Arven correctly consumes the turn's Supporter action:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm2-125
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(EngineTestAccess::play_arven(engine));
  assert(EngineTestAccess::state(engine).supporter_used);
  assert(std::count(EngineTestAccess::state(engine).hand.begin(),
                    EngineTestAccess::state(engine).hand.end(), Card::FieldBlower) == 1);
  assert(EngineTestAccess::play_field_blower(engine));
}

void test_garbodor_forest_seal_stone_remains_usable() {
  using namespace sim;
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(280808);
  const Scenario scenario{"garbodor-shake-ability-lock/go-second",
                          DciProfile::StrictJit, LockMode::None, false, 5};
  Engine engine(scenario, recipe, rng);
  State state = minimal_garbodor_state(1);
  state.active = Pokemon{Card::RegidragoV, 0};
  state.active->tool = Tool::ForestSealStone;
  state.deck = {Card::RegidragoVstar, Card::Grass, Card::Fire,
                Card::MegaDragonite, Card::MysteriousTreasure};
  EngineTestAccess::set_state(engine, std::move(state));

  // Star Alchemy is the Tool's Ability that the attached Pokemon V may use. The
  // repository's existing ruling keeps it outside Pokemon-Ability suppression:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  assert(EngineTestAccess::garbodor_locked(engine));
  assert(EngineTestAccess::use_fss(engine));
}

void test_garbodor_scenarios_are_registered() {
  const auto scenarios = sim::all_scenarios_with_garbodor();
  const auto contains = [&scenarios](const std::string& label) {
    return std::any_of(scenarios.begin(), scenarios.end(), [&label](const sim::Scenario& scenario) {
      return scenario.label == label;
    });
  };
  assert(contains("garbodor-shake-ability-lock/go-first"));
  assert(contains("garbodor-shake-ability-lock/go-second"));
}

}  // namespace

int main() {
  test_ready_state_holds_hisuian_heavy_ball();
  test_ready_state_holds_field_blower();
  test_field_blower_restores_live_wonder_tag_route();
  test_field_blower_restores_live_legacy_star_route();
  test_garbodor_boost_shake_timing_by_seat();
  test_garbodor_field_blower_unlock_is_current_turn_only();
  test_garbodor_dark_asset_six_card_boundary_is_live();
  test_garbodor_only_actual_item_lock_blocks_field_blower();
  test_garbodor_arven_searches_field_blower_for_live_unlock();
  test_garbodor_forest_seal_stone_remains_usable();
  test_garbodor_scenarios_are_registered();
  return 0;
}
