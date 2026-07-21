#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe appletun_recipe() {
  // The registered Pineco recipe is the repository's valid 60-card Appletun list:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#named-deck-recipes
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L230-L253
  // https://api.pokemontcg.io/v2/cards/sv8-140
  return sim::pineco_recipe();
}

sim::State payload_only_state(std::vector<sim::Card> hand,
                              std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = std::move(hand);
  state.deck = std::move(deck);
  state.supporter_used = true;
  return state;
}

void test_known_appletun_to_earthen_vessel_route() {
  const sim::Scenario scenario{"issue-1288-known-positive",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{128801};
  sim::Engine engine(scenario, appletun_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::Grant,
                   sim::Card::EarthenVessel},
                  {sim::Card::Appletun, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Mysterious Treasure discards one card and searches a Dragon Pokémon. Appletun
  // sv8-140 is a Stage 1 Dragon and a modeled Apex Dragon payload. Earthen Vessel
  // survives the first Item, discards Appletun, and retains Grass as a legal target:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1288
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Mysterious Treasure should fetch Appletun for the live Vessel outlet.");
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "Earthen Vessel should discard Appletun and search Grass.");

  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::MysteriousTreasure) &&
             contains(result.discard, sim::Card::Grant) &&
             contains(result.discard, sim::Card::Appletun) &&
             contains(result.discarded_this_turn, sim::Card::Appletun) &&
             contains(result.hand, sim::Card::Grass) &&
             !contains(result.deck, sim::Card::Appletun),
         "The exact Treasure-Appletun-Vessel route must preserve every zone transition.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "Discarding Appletun through Vessel should complete strict-JIT payload timing.");
}

void test_holds_without_surviving_outlet() {
  const sim::Scenario scenario{"issue-1288-no-outlet",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{128802};
  sim::Engine engine(scenario, appletun_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::Grant},
                  {sim::Card::Appletun, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Searching Appletun without a distinct surviving discard outlet cannot put it
  // into discard this turn, so strict-JIT must preserve Treasure and Grant:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure must hold when no post-fetch payload outlet survives.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 2U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The no-outlet control must preserve the full state.");
}

void test_holds_when_vessel_has_no_post_search_target() {
  const sim::Scenario scenario{"issue-1288-targetless-vessel",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{128803};
  sim::Engine engine(scenario, appletun_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::Grant,
                   sim::Card::EarthenVessel},
                  {sim::Card::Appletun, sim::Card::Dipplin}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // After Treasure removes Appletun, Earthen Vessel has no Basic Energy target.
  // A known targetless Trainer cannot justify spending the first Item and its cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/1288
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure must hold when Vessel has no post-Appletun Energy target.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 3U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The targetless-Vessel control must preserve both Items, Grant, and deck.");
}

void test_item_lock_blocks_route() {
  const sim::Scenario scenario{"issue-1288-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  std::mt19937_64 rng{128805};
  sim::Engine engine(scenario, appletun_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::Grant,
                   sim::Card::EarthenVessel},
                  {sim::Card::Appletun, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Full Item lock prohibits both Mysterious Treasure and Earthen Vessel from hand:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#full-item-lock
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Item lock must block the Treasure-Appletun-Vessel route.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 3U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The Item-lock control must preserve the full state.");
}

}  // namespace

int main() {
  try {
    test_known_appletun_to_earthen_vessel_route();
    test_holds_without_surviving_outlet();
    test_holds_when_vessel_has_no_post_search_target();
    test_item_lock_blocks_route();
    std::cout << "Issue 1288 Mysterious Treasure Appletun outlet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
