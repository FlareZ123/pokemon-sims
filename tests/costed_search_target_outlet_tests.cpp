#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(true); }
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

void test_treasure_holds_before_off_target_quick_ball() {
  const sim::Scenario scenario{"treasure-off-target-quick",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5701};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                   sim::Card::Dipplin},
                  {sim::Card::MegaDragonite, sim::Card::Dragapult}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Treasure would fetch Mega Dragonite ex and leave only the Evolution Pokémon
  // Dragapult ex. Quick Ball searches only Basic Pokémon, so the promised second
  // Item is known to have no target and the first search must preserve its resources:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure must hold when no Basic target remains for Quick Ball.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 3U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The dead Quick Ball route must preserve both Items, the cost, and deck.");
}

void test_quick_ball_holds_before_off_target_treasure() {
  const sim::Scenario scenario{"quick-off-target-treasure",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5702};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                   sim::Card::Dipplin},
                  {sim::Card::DialgaGX, sim::Card::Serena}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Quick Ball would fetch Basic Dialga-GX and leave Serena. Mysterious Treasure
  // searches only Psychic or Dragon Pokémon, so Serena cannot support the promised
  // current-turn discard continuation:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must hold when no Treasure target remains.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 3U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The dead Treasure route must preserve both Items, the cost, and deck.");
}

void test_treasure_holds_before_off_target_vessel() {
  const sim::Scenario scenario{"treasure-off-target-vessel",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5703};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                   sim::Card::Dipplin},
                  {sim::Card::MegaDragonite, sim::Card::Dipplin}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Treasure would fetch Mega Dragonite ex and leave Dipplin. Earthen Vessel
  // searches only Basic Energy, so the remaining Pokémon is not a legal target:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure must hold when no Basic Energy remains for Vessel.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 3U && result.discard.empty() &&
             result.deck.size() == 2U,
         "The dead Vessel route must preserve both Items, the cost, and deck.");
}

void test_quick_ball_keeps_live_treasure_target() {
  const sim::Scenario scenario{"quick-live-treasure-target",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5704};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                   sim::Card::Dipplin},
                  {sim::Card::DialgaGX, sim::Card::Dipplin}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Dipplin is a legal Dragon target for Mysterious Treasure after Quick Ball
  // removes Dialga-GX:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv6-127
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should keep a live Treasure continuation.");
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure should discard Dialga-GX and search Dipplin.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::DialgaGX) &&
             contains(result.discarded_this_turn, sim::Card::DialgaGX) &&
             contains(result.hand, sim::Card::Dipplin),
         "The live Treasure control must discard Dialga-GX and take Dipplin.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The live Quick Ball into Treasure route should satisfy strict-JIT timing.");
}

void test_treasure_keeps_live_quick_ball_target() {
  const sim::Scenario scenario{"treasure-live-quick-target",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5705};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                   sim::Card::Dipplin},
                  {sim::Card::MegaDragonite, sim::Card::RegidragoV}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Regidrago V is a legal Basic target for Quick Ball after Treasure removes the
  // Evolution payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure should keep a live Quick Ball continuation.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should discard Mega Dragonite ex and search Regidrago V.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::MegaDragonite) &&
             contains(result.discarded_this_turn, sim::Card::MegaDragonite) &&
             contains(result.hand, sim::Card::RegidragoV),
         "The live Quick Ball control must discard the payload and take Regidrago V.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The live Treasure into Quick Ball route should satisfy strict-JIT timing.");
}

void test_treasure_keeps_live_vessel_target() {
  const sim::Scenario scenario{"treasure-live-vessel-target",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{5706};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, payload_only_state(
                  {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                   sim::Card::Dipplin},
                  {sim::Card::MegaDragonite, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Grass is a legal Basic Energy target after Treasure fetches the Dragon payload.
  // Vessel may discard that payload and finish its printed search:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Treasure should keep a live Vessel continuation.");
  expect(sim::EngineTestAccess::play_earthen_vessel(engine),
         "Vessel should discard Mega Dragonite ex and search Grass.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::MegaDragonite) &&
             contains(result.discarded_this_turn, sim::Card::MegaDragonite) &&
             contains(result.hand, sim::Card::Grass),
         "The live Vessel control must discard the payload and take Grass.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The live Treasure into Vessel route should satisfy strict-JIT timing.");
}

}  // namespace

int main() {
  try {
    test_treasure_holds_before_off_target_quick_ball();
    test_quick_ball_holds_before_off_target_treasure();
    test_treasure_holds_before_off_target_vessel();
    test_quick_ball_keeps_live_treasure_target();
    test_treasure_keeps_live_quick_ball_target();
    test_treasure_keeps_live_vessel_target();
    std::cout << "costed search target outlet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
