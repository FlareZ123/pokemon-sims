#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_ultra_ball(Engine& engine) { return engine.play_ultra_ball(false); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

namespace {

void test_ultra_ball_and_dipplin_pay_the_cost() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-redundant-cost", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(151);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::Dipplin};
  state.deck = {Card::RegidragoVstar};

  // Ultra Ball discards 2 other cards from hand before it searches for a Pokémon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Regidrago VSTAR is a Pokémon and is a legal Ultra Ball target: https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoVstar) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 2);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Dipplin) == 1);
}

void test_two_other_ultra_balls_pay_the_cost() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-three-copy-cost", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(152);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::UltraBall};
  state.deck = {Card::RegidragoVstar};

  // Each of the two discarded copies is another card from hand relative to the
  // Ultra Ball being played: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoVstar) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 3);
}

void test_gladion_preserves_a_payable_three_copy_ultra_ball_route() {
  using namespace sim;
  const Scenario scenario{"gladion-three-copy-ultra-ball-route", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(153);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1};
  state.hand = {Card::Gladion, Card::UltraBall, Card::UltraBall, Card::UltraBall};
  state.deck = {Card::RegidragoVstar};
  state.prizes = {Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma, Card::Channeler};
  EngineTestAccess::set_deck_seen(engine);

  // The three-copy Ultra Ball hand has a payable direct VSTAR search, so the one
  // Supporter play must be preserved instead of spending Gladion: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Gladion exchanges itself for a Prize card and cannot improve this known Prize set: https://api.pokemontcg.io/v2/cards/sm4-95
  assert(!EngineTestAccess::play_gladion(engine));
  assert(!state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Gladion) == 1);
}

}  // namespace

int main() {
  test_ultra_ball_and_dipplin_pay_the_cost();
  test_two_other_ultra_balls_pay_the_cost();
  test_gladion_preserves_a_payable_three_copy_ultra_ball_route();
  return 0;
}
