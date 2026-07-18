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
  }
  static void play_items_until_stable(Engine& engine, const bool permit_payload) {
    engine.play_items_until_stable(permit_payload);
  }
  static bool play_field_blower(Engine& engine) {
    return engine.play_field_blower();
  }
};

}  // namespace sim

namespace {

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
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
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

}  // namespace

int main() {
  test_ready_state_holds_hisuian_heavy_ball();
  test_ready_state_holds_field_blower();
  test_field_blower_restores_live_wonder_tag_route();
  test_field_blower_restores_live_legacy_star_route();
  return 0;
}
