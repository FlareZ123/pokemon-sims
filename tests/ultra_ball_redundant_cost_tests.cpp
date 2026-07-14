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
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
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

void test_payload_probe_requires_a_second_ultra_ball_after_the_first_play() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-second-copy-required", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(318);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::UltraBall, Card::HisuianHeavyBall};
  state.deck = {Card::MegaDragonite};
  state.prizes = {Card::Grass, Card::Fire, Card::Dipplin, Card::Guzma, Card::Channeler, Card::Lusamine};
  EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball must itself remain in hand for the claimed second play and then
  // discard two other cards: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Hisuian Heavy Ball cannot independently discard the fetched payload: https://api.pokemontcg.io/v2/cards/swsh10-146
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::UltraBall) == 3);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::HisuianHeavyBall) == 1);
  assert(std::count(state.deck.begin(), state.deck.end(), Card::MegaDragonite) == 1);
  assert(state.discard.empty());
}

void test_ultra_ball_preserves_resources_when_payload_is_the_final_deck_card() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-final-card-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(448);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin,
                Card::RegidragoVstar};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball would remove the final deck card before Mysterious Treasure could
  // begin its required search, so neither Item nor either discard cost should be spent:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.deck == std::vector<Card>{Card::MegaDragonite});
  assert(state.discard.empty());
  assert(std::count(state.hand.begin(), state.hand.end(), Card::UltraBall) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MysteriousTreasure) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Dipplin) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoVstar) == 1);
}

void test_ultra_ball_uses_search_item_outlet_when_one_card_remains_after_payload() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-payload-leaves-search-card", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(449);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin,
                Card::RegidragoVstar};
  state.deck = {Card::MegaDragonite, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // One card remains after Ultra Ball fetches the Dragon, so Mysterious Treasure can
  // legally discard that payload and begin its search, even when it finds no target:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
  assert(state.deck == std::vector<Card>{Card::Grass});
  assert(EngineTestAccess::play_mysterious_treasure(engine));
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discarded_this_turn.begin(), state.discarded_this_turn.end(),
                    Card::MegaDragonite) == 1);
}

void test_ultra_ball_allows_serena_after_fetching_the_final_deck_card() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-final-card-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(450);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::Serena, Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Serena's draw mode discards from hand and does not require another deck search,
  // so it remains a live strict-JIT outlet after Ultra Ball removes the final card:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(state.deck.empty());
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
}

}  // namespace

int main() {
  test_ultra_ball_and_dipplin_pay_the_cost();
  test_two_other_ultra_balls_pay_the_cost();
  test_gladion_preserves_a_payable_three_copy_ultra_ball_route();
  test_payload_probe_requires_a_second_ultra_ball_after_the_first_play();
  test_ultra_ball_preserves_resources_when_payload_is_the_final_deck_card();
  test_ultra_ball_uses_search_item_outlet_when_one_card_remains_after_payload();
  test_ultra_ball_allows_serena_after_fetching_the_final_deck_card();
  return 0;
}
