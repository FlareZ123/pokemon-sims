#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool has_live_one_discard_hand_payload_line(const Engine& engine) {
    return engine.has_live_one_discard_hand_payload_line();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_duplicate_mysterious_treasure_is_legal_cost() {
  const sim::Scenario scenario{"duplicate-mysterious-treasure-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{83};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoV};
  // Mysterious Treasure discards a card from hand before searching a Psychic or
  // Dragon Pokémon, so the other copy is a legal cost: https://api.pokemontcg.io/v2/cards/sm6-113
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "Mysterious Treasure should use the other copy as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Mysterious Treasure should search the missing Dragon Pokémon");
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 2,
         "the played Mysterious Treasure and its duplicate cost should both be discarded");
  expect(after.deck.empty(), "the searched Regidrago V should leave the deck");
}

void test_mysterious_treasure_can_discard_unpayable_ultra_ball() {
  const sim::Scenario scenario{"mysterious-treasure-ultra-ball-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{84};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoV};
  // Mysterious Treasure needs one hand discard, while Ultra Ball needs two other
  // cards and is not payable from this two-card hand:
  // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "Mysterious Treasure should use held Ultra Ball as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Mysterious Treasure should search Regidrago V after discarding Ultra Ball");
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 1,
         "the played Mysterious Treasure should be discarded");
  expect(count(after.discard, sim::Card::UltraBall) == 1,
         "the held Ultra Ball should be discarded as the cost");
  expect(after.deck.empty(), "the searched Regidrago V should leave the deck");
}

void test_duplicate_supporter_is_legal_cost(const sim::Card supporter,
                                            const std::uint64_t seed) {
  const sim::Scenario scenario{"duplicate-supporter-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, supporter, supporter};
  state.deck = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Mysterious Treasure may discard any other hand card, and strict DCI lists a
  // duplicate as potential discard fuel while one copy remains available:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // A player may use only one Supporter per turn, so preserving one copy retains
  // the complete Supporter option: https://www.pokemon.com/us/pokemon-tcg/rules
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "Mysterious Treasure should use one duplicated Supporter as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure should search Regidrago VSTAR");
  expect(count(after.hand, supporter) == 1,
         "one Supporter copy should remain in hand");
  expect(count(after.discard, supporter) == 1,
         "one duplicated Supporter should pay the discard cost");
  expect(count(after.discard, sim::Card::MysteriousTreasure) == 1,
         "the played Mysterious Treasure should enter discard");
  expect(after.deck.empty(), "the searched Regidrago VSTAR should leave the deck");
}

void test_singleton_supporter_is_preserved(const sim::Card supporter,
                                           const std::uint64_t seed) {
  const sim::Scenario scenario{"singleton-supporter-hold", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, supporter};
  state.deck = {sim::Card::RegidragoVstar};
  const std::vector<sim::Card> original_hand = state.hand;
  const std::vector<sim::Card> original_deck = state.deck;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // The duplicate policy requires two copies. A singleton Arven, Crispin, or Gladion
  // remains protected under strict DCI:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Card texts: https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "Mysterious Treasure should preserve a singleton Supporter under strict DCI");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.hand == original_hand, "the singleton Supporter hand should remain unchanged");
  expect(after.deck == original_deck, "the unplayed search should leave the deck unchanged");
  expect(after.discard.empty(), "no card should enter discard when the cost is unavailable");
}

void test_known_no_target_mysterious_treasure_preserves_payload() {
  const sim::Scenario scenario{"mysterious-treasure-known-no-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{554};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Arven};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // K1 proves that no Psychic or Dragon Pokémon remains, so the Trainer is known
  // to have no effect and its discard requirement cannot be paid:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine, true),
         "known zero-target Mysterious Treasure should be held");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.hand == std::vector<sim::Card>{sim::Card::MysteriousTreasure,
                                               sim::Card::MegaDragonite},
         "Mysterious Treasure and the payload should remain in hand");
  expect(after.deck == std::vector<sim::Card>{sim::Card::Arven},
         "the known non-target deck should remain unchanged");
  expect(after.discard.empty(), "no play cost should enter discard");
}

void test_known_no_target_mysterious_treasure_is_not_a_tate_route() {
  const sim::Scenario scenario{"tate-mysterious-known-no-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{555};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::TateLiza, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Arven};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Tate planning must reject the same dead continuation because Mysterious Treasure
  // has no legal Psychic or Dragon target in the inspected deck:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::has_live_one_discard_hand_payload_line(engine),
         "Tate should not preserve a route through known-dead Mysterious Treasure");
}

}  // namespace

int main() {
  try {
    test_duplicate_mysterious_treasure_is_legal_cost();
    test_mysterious_treasure_can_discard_unpayable_ultra_ball();
    test_duplicate_supporter_is_legal_cost(sim::Card::Arven, 6071);
    test_duplicate_supporter_is_legal_cost(sim::Card::Crispin, 6072);
    test_duplicate_supporter_is_legal_cost(sim::Card::Gladion, 6073);
    test_singleton_supporter_is_preserved(sim::Card::Arven, 6074);
    test_singleton_supporter_is_preserved(sim::Card::Crispin, 6075);
    test_singleton_supporter_is_preserved(sim::Card::Gladion, 6076);
    test_known_no_target_mysterious_treasure_preserves_payload();
    test_known_no_target_mysterious_treasure_is_not_a_tate_route();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
