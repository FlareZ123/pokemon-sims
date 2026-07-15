#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload = false) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool second_ultra_ball_can_discard_fetched_payload(Engine& engine) {
    return engine.second_ultra_ball_can_discard_fetched_payload();
  }
  static bool ultra_ball_can_discard_fetched_incense_payload(Engine& engine) {
    return engine.ultra_ball_can_discard_fetched_incense_payload();
  }
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

void test_k1_ultra_ball_preserves_payload_when_no_pokemon_target_exists() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-known-no-pokemon", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(555);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MegaDragonite, Card::Dipplin};
  state.deck = {Card::Arven};
  EngineTestAccess::set_deck_seen(engine);

  // K1 proves that Ultra Ball has no Pokémon target. Its two-card play requirement
  // cannot be paid merely to put a Dragon payload into discard:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  assert(!EngineTestAccess::play_ultra_ball(engine, true));
  assert(state.hand == std::vector<Card>({Card::UltraBall, Card::MegaDragonite, Card::Dipplin}));
  assert(state.deck == std::vector<Card>({Card::Arven}));
  assert(state.discard.empty());
  assert(state.discarded_this_turn.empty());
}

void test_k1_ultra_ball_payload_cost_remains_legal_with_a_pokemon_target() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-known-pokemon", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(556);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MegaDragonite, Card::Dipplin};
  state.deck = {Card::MawileGX};
  EngineTestAccess::set_deck_seen(engine);

  // Mawile-GX is a Pokémon, so the known Ultra Ball search remains legal after its
  // two-card requirement is paid: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  assert(EngineTestAccess::play_ultra_ball(engine, true));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MawileGX) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Dipplin) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 1);
}

void test_k0_ultra_ball_may_legally_attempt_an_unknown_search() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-unknown-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(557);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MegaDragonite, Card::Dipplin};
  state.deck = {Card::Arven};

  // Before legal deck inspection, the fixed-list Pokémon remain plausible under K0,
  // so Ultra Ball may be played and discover that the search has no target:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  assert(EngineTestAccess::play_ultra_ball(engine, true));
  assert(state.hand.empty());
  assert(state.deck == std::vector<Card>({Card::Arven}));
  assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Dipplin) == 1);
}

void test_k1_ultra_ball_route_probes_reject_a_known_non_pokemon_deck() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-known-dead-probes", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(558);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::UltraBall,
                Card::EvolutionIncense, Card::EvolutionIncense,
                Card::Dipplin, Card::Grass};
  state.deck = {Card::Arven};
  EngineTestAccess::set_deck_seen(engine);

  // Route probes must apply the same known-target legality as the eventual Ultra Ball
  // play and cannot promise a discard outlet through a known zero-target search:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  assert(!EngineTestAccess::second_ultra_ball_can_discard_fetched_payload(engine));
  assert(!EngineTestAccess::ultra_ball_can_discard_fetched_incense_payload(engine));
}

void test_k1_ultra_ball_rejects_off_target_payload_outlets() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-off-target-outlet", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();

  const auto rejected = [&](const Card outlet, std::vector<Card> deck,
                            const std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    Engine engine(scenario, recipe, rng);
    State& state = EngineTestAccess::state(engine);
    state.turn = 2;
    state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
    state.hand = {Card::UltraBall, outlet, Card::Dipplin, Card::Dipplin};
    state.deck = std::move(deck);
    EngineTestAccess::set_deck_seen(engine);
    const std::vector<Card> original_hand = state.hand;
    const std::vector<Card> original_deck = state.deck;
    assert(!EngineTestAccess::play_ultra_ball(engine, true));
    assert(state.hand == original_hand);
    assert(state.deck == original_deck);
    assert(state.discard.empty());
  };

  // Ultra Ball would fetch Mega Dragonite ex. The remaining card is off-class for
  // the promised second Item, so the three-card Ultra Ball play must be preserved:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  rejected(Card::MysteriousTreasure, {Card::MegaDragonite, Card::Serena}, 5761);
  rejected(Card::QuickBall, {Card::MegaDragonite, Card::Dragapult}, 5762);
  rejected(Card::EarthenVessel, {Card::MegaDragonite, Card::Dipplin}, 5763);
}

void test_k1_ultra_ball_accepts_live_payload_outlets() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-live-outlet", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();

  const auto accepted = [&](const Card outlet, std::vector<Card> deck,
                            const std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    Engine engine(scenario, recipe, rng);
    State& state = EngineTestAccess::state(engine);
    state.turn = 2;
    state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
    state.hand = {Card::UltraBall, outlet, Card::Dipplin, Card::Dipplin};
    state.deck = std::move(deck);
    EngineTestAccess::set_deck_seen(engine);
    assert(EngineTestAccess::play_ultra_ball(engine, true));
    assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
    assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 1);
  };

  // Dipplin is Dragon, Dialga-GX is Basic, and Grass is Basic Energy, so each
  // post-Ultra-Ball search remains legal:
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sve-1
  accepted(Card::MysteriousTreasure, {Card::MegaDragonite, Card::Dipplin}, 5764);
  accepted(Card::QuickBall, {Card::MegaDragonite, Card::DialgaGX}, 5765);
  accepted(Card::EarthenVessel, {Card::MegaDragonite, Card::Grass}, 5766);
}

void test_ultra_ball_preserves_serena_and_k0_payload_routes() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-serena-and-k0", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();

  {
    std::mt19937_64 rng(5767);
    Engine engine(scenario, recipe, rng);
    State& state = EngineTestAccess::state(engine);
    state.turn = 2;
    state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
    state.hand = {Card::UltraBall, Card::Serena, Card::Dipplin, Card::Dipplin};
    state.deck = {Card::MegaDragonite, Card::Dragapult};
    EngineTestAccess::set_deck_seen(engine);
    // Serena can discard the fetched payload without another deck search:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    assert(EngineTestAccess::play_ultra_ball(engine, true));
  }

  {
    std::mt19937_64 rng(5768);
    Engine engine(scenario, recipe, rng);
    State& state = EngineTestAccess::state(engine);
    state.turn = 2;
    state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
    state.hand = {Card::UltraBall, Card::QuickBall, Card::Dipplin, Card::Dipplin};
    state.deck = {Card::MegaDragonite, Card::Dragapult};
    // K0 cannot inspect the remaining card class before the legal Ultra Ball search:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    assert(EngineTestAccess::play_ultra_ball(engine, true));
  }
}

}  // namespace

int main() {
  test_ultra_ball_and_dipplin_pay_the_cost();
  test_two_other_ultra_balls_pay_the_cost();
  test_gladion_preserves_a_payable_three_copy_ultra_ball_route();
  test_payload_probe_requires_a_second_ultra_ball_after_the_first_play();
  test_k1_ultra_ball_preserves_payload_when_no_pokemon_target_exists();
  test_k1_ultra_ball_payload_cost_remains_legal_with_a_pokemon_target();
  test_k0_ultra_ball_may_legally_attempt_an_unknown_search();
  test_k1_ultra_ball_route_probes_reject_a_known_non_pokemon_deck();
  test_k1_ultra_ball_rejects_off_target_payload_outlets();
  test_k1_ultra_ball_accepts_live_payload_outlets();
  test_ultra_ball_preserves_serena_and_k0_payload_routes();
  return 0;
}
