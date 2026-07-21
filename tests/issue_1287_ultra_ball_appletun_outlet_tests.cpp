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
  static bool play_ultra_ball(Engine& engine) { return engine.play_ultra_ball(true); }
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

void test_ultra_ball_fetches_appletun_for_live_treasure() {
  const sim::Scenario scenario{"issue-1287-treasure-positive",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{128701};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::UltraBall, sim::Card::Grant,
                   sim::Card::WishfulBaton, sim::Card::MysteriousTreasure},
                  {sim::Card::Appletun, sim::Card::RegidragoV}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball may search Appletun, and Mysterious Treasure remains playable because
  // Regidrago V is a separate Dragon target after Appletun leaves the known deck:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/issues/1287
  expect(sim::EngineTestAccess::play_ultra_ball(engine),
         "Ultra Ball should fetch Appletun when Treasure keeps a legal target.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Appletun),
         "Ultra Ball should put Appletun into hand.");
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure should discard Appletun and search Regidrago V.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::Appletun) &&
             contains(result.discarded_this_turn, sim::Card::Appletun) &&
             contains(result.hand, sim::Card::RegidragoV),
         "The positive route must discard Appletun and retain the legal target.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The positive route should satisfy strict-JIT payload timing.");
}

void test_cross_singleton_discards_targetless_treasure() {
  const sim::Scenario scenario{"issue-1287-cross-singleton",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{128702};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::UltraBall, sim::Card::Grant,
                   sim::Card::MysteriousTreasure, sim::Card::EarthenVessel},
                  {sim::Card::Appletun, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Once Ultra Ball removes Appletun, Mysterious Treasure has no Psychic or Dragon
  // target. Earthen Vessel still has Grass Energy, so Treasure is the correct second
  // Ultra Ball cost and Vessel is the live Appletun discard outlet:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/1287
  expect(sim::EngineTestAccess::play_ultra_ball(engine),
         "Ultra Ball should preserve the live Vessel outlet.");
  const sim::State& after_ultra = sim::EngineTestAccess::state(engine);
  expect(contains(after_ultra.discard, sim::Card::MysteriousTreasure) &&
             contains(after_ultra.hand, sim::Card::EarthenVessel) &&
             contains(after_ultra.hand, sim::Card::Appletun),
         "Ultra Ball must spend targetless Treasure and preserve Vessel.");
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "Vessel should discard Appletun and search Grass Energy.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::Appletun) &&
             contains(result.discarded_this_turn, sim::Card::Appletun) &&
             contains(result.hand, sim::Card::Grass),
         "The cross-singleton route must finish through Vessel.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The Vessel continuation should satisfy strict-JIT payload timing.");
}

void test_ultra_ball_holds_when_treasure_has_no_post_search_target() {
  const sim::Scenario scenario{"issue-1287-targetless-control",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{128703};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::UltraBall, sim::Card::Grant,
                   sim::Card::WishfulBaton, sim::Card::MysteriousTreasure},
                  {sim::Card::Appletun, sim::Card::Serena}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Appletun cannot count as Mysterious Treasure's surviving target after Ultra Ball
  // has already removed it, and Serena is not a Psychic or Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/1287
  expect(!sim::EngineTestAccess::play_ultra_ball(engine),
         "Ultra Ball must hold when the promised Treasure has no target.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 4U && result.deck.size() == 2U &&
             result.discard.empty(),
         "The targetless control must preserve the entire state.");
}

void test_item_lock_blocks_ultra_ball_route() {
  const sim::Scenario scenario{"issue-1287-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{128704};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = payload_only_state(
      {sim::Card::UltraBall, sim::Card::Grant,
       sim::Card::WishfulBaton, sim::Card::MysteriousTreasure},
      {sim::Card::Appletun, sim::Card::RegidragoV});
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Full Item lock prevents Ultra Ball regardless of its otherwise legal target and costs:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1287
  expect(!sim::EngineTestAccess::play_ultra_ball(engine),
         "Item lock must block the Ultra Ball route.");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_fetches_appletun_for_live_treasure();
    test_cross_singleton_discards_targetless_treasure();
    test_ultra_ball_holds_when_treasure_has_no_post_search_target();
    test_item_lock_blocks_ultra_ball_route();
    std::cout << "Issue 1287 Ultra Ball Appletun outlet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
