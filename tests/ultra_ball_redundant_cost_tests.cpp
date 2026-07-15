#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload = false) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool second_ultra_ball_can_discard_fetched_payload(Engine& engine) {
    return engine.second_ultra_ball_can_discard_fetched_payload();
  }
  static bool second_ultra_ball_has_post_payload_target(Engine& engine) {
    return engine.second_ultra_ball_has_post_payload_target();
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

void test_k1_ultra_ball_preserves_final_payload_before_search_item_outlet() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-final-payload-dead-outlet", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(4481);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin,
                Card::RegidragoVstar};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball would remove the final deck card before Mysterious Treasure could
  // begin its required search. Preserve both Items and both Ultra Ball costs:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.hand == std::vector<Card>({Card::UltraBall, Card::MysteriousTreasure,
                                         Card::Dipplin, Card::RegidragoVstar}));
  assert(state.deck == std::vector<Card>({Card::MegaDragonite}));
  assert(state.discard.empty());
}

void test_k1_ultra_ball_keeps_target_legal_search_item_continuation() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-live-treasure-continuation", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(4482);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::RegidragoVstar};
  EngineTestAccess::set_deck_seen(engine);

  // After Ultra Ball fetches Mega Dragonite ex, Regidrago VSTAR remains a legal
  // Dragon target for Mysterious Treasure, which may discard the fetched payload:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(EngineTestAccess::play_mysterious_treasure(engine));
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoVstar) == 1);
}

void test_k1_ultra_ball_keeps_serena_final_card_continuation() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-final-payload-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(4483);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::Serena, Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Serena can discard the fetched Dragon without a second deck search, so Ultra Ball
  // remains a live final-card continuation:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(state.deck.empty());
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 1);
}

void test_k0_ultra_ball_preserves_unknown_final_card_routes() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-k0-final-card", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(4484);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::MysteriousTreasure, Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite};

  // K0 cannot inspect the remaining card identity before the legal Ultra Ball search,
  // so fixed-list ordinary Pokémon targets remain plausible and the play is preserved:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(state.deck.empty());
  assert(std::count(state.hand.begin(), state.hand.end(), Card::MegaDragonite) == 1);
}

void test_k1_second_ultra_route_rejects_a_zero_target_post_payload_deck() {
  using namespace sim;
  const Scenario scenario{"second-ultra-post-payload-zero-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(590);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::Dipplin,
                Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Grass};
  EngineTestAccess::set_deck_seen(engine);

  // The first Ultra Ball would fetch Mega Dragonite ex, leaving only Grass Energy.
  // A second Ultra Ball would then be known to have no Pokémon target, so the route
  // must preserve both Items and all three discardable cards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/base1-99
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  assert(!EngineTestAccess::second_ultra_ball_has_post_payload_target(engine));
  assert(!EngineTestAccess::play_ultra_ball(engine));
  assert(state.hand == std::vector<Card>({Card::UltraBall, Card::UltraBall,
                                         Card::Dipplin, Card::Dipplin, Card::Dipplin}));
  assert(state.deck == std::vector<Card>({Card::MegaDragonite, Card::Grass}));
  assert(state.discard.empty());
}

void test_k1_second_ultra_route_accepts_a_remaining_pokemon_target() {
  using namespace sim;
  const Scenario scenario{"second-ultra-post-payload-live-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(591);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::Dipplin,
                Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Dipplin};
  EngineTestAccess::set_deck_seen(engine);

  // After Mega Dragonite ex is removed, Dipplin remains a Pokémon target for the
  // second Ultra Ball, so the target-legality portion of the route is live:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-127
  assert(EngineTestAccess::second_ultra_ball_has_post_payload_target(engine));
}

void test_k0_second_ultra_route_preserves_hidden_deck_uncertainty() {
  using namespace sim;
  const Scenario scenario{"second-ultra-k0-unknown-target", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(592);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::Dipplin,
                Card::Dipplin, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::Grass};

  // K0 cannot inspect the remaining deck identities before a legal search resolves,
  // so the fixed-list second-target possibility remains live:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  assert(EngineTestAccess::second_ultra_ball_has_post_payload_target(engine));
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
  test_k1_ultra_ball_preserves_final_payload_before_search_item_outlet();
  test_k1_ultra_ball_keeps_target_legal_search_item_continuation();
  test_k1_ultra_ball_keeps_serena_final_card_continuation();
  test_k0_ultra_ball_preserves_unknown_final_card_routes();
  test_k1_second_ultra_route_rejects_a_zero_target_post_payload_deck();
  test_k1_second_ultra_route_accepts_a_remaining_pokemon_target();
  test_k0_second_ultra_route_preserves_hidden_deck_uncertainty();
  return 0;
}
