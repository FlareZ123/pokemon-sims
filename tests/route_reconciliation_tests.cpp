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
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) { return engine.play_mysterious_treasure(permit_payload); }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) { return engine.play_quick_ball(permit_payload); }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) { return engine.play_ultra_ball(permit_payload); }
  static bool play_evolution_incense(Engine& engine, const bool permit_payload) { return engine.play_evolution_incense(permit_payload); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(const sim::Scenario& scenario, const std::uint64_t seed) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 unused_rng{1};
  (void)unused_rng;
  std::mt19937_64 rng{seed};
  return sim::Engine(scenario, recipe, rng);
}

void test_ultra_ball_requires_two_distinct_costs() {
  // Ultra Ball requires discarding 2 other cards from hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"ultra-ball-distinct-cost", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{54};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_ultra_ball(engine, false),
         "Ultra Ball must not use the same singleton card twice for its two discard costs.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::UltraBall) && contains(after.hand, sim::Card::Dipplin) &&
             after.discard.empty() && contains(after.deck, sim::Card::RegidragoV),
         "An unpayable Ultra Ball must leave the exact state unchanged.");
}

void test_evolution_incense_hands_payload_to_payable_ultra_ball() {
  // Evolution Incense may find an Evolution Pokémon, and Ultra Ball may then discard
  // that payload plus a distinct card: https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"evolution-incense-ultra-ball", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{112};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::UltraBall, sim::Card::Dipplin};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_evolution_incense(engine, true),
         "Evolution Incense should fetch the current-turn payload for a payable Ultra Ball.");
  expect(sim::EngineTestAccess::play_ultra_ball(engine, true),
         "Ultra Ball should discard the fetched payload with Dipplin as its other cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "The Evolution Incense route must leave the Dragon payload in this turn's discard.");
}

void test_two_ultra_ball_payload_line_requires_second_cost() {
  // Each Ultra Ball independently needs 2 other cards from hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"two-ultra-ball-capacity", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{115};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.bench = {{sim::Card::RegidragoV, 1}, {sim::Card::RegidragoV, 1},
                 {sim::Card::MawileGX, 1}, {sim::Card::LatiasEx, 1}, {sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::UltraBall, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::RegidragoV};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_ultra_ball(engine, true),
         "The first Ultra Ball must be held when a second Ultra Ball would lack a distinct second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::UltraBall) == 2 &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.deck, sim::Card::MegaDragonite),
         "Rejecting the unpayable two-Ultra-Ball continuation must preserve state.");
}

sim::State prized_vstar_state(std::vector<sim::Card> hand) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = std::move(hand);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  return state;
}

void test_gladion_uses_prized_vstar_only_when_item_cost_is_unpayable() {
  // Mysterious Treasure needs one discard and Ultra Ball needs two other card costs;
  // Gladion exchanges itself for a Prize card: https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-vstar-cost", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  std::mt19937_64 unpayable_rng{116};
  sim::Engine unpayable(scenario, recipe, unpayable_rng);
  sim::EngineTestAccess::set_state(unpayable, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure}));
  expect(sim::EngineTestAccess::play_gladion(unpayable) &&
             contains(sim::EngineTestAccess::state(unpayable).hand, sim::Card::RegidragoVstar),
         "Gladion must recover the prized VSTAR when Mysterious Treasure cannot pay its cost.");

  std::mt19937_64 payable_rng{117};
  sim::Engine payable(scenario, recipe, payable_rng);
  sim::EngineTestAccess::set_state(payable, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure, sim::Card::Dipplin}));
  expect(!sim::EngineTestAccess::play_gladion(payable) &&
             contains(sim::EngineTestAccess::state(payable).hand, sim::Card::Gladion),
         "Gladion must remain available when Mysterious Treasure has a legal discard cost.");
}

void test_gladion_is_held_for_arven_forest_seal_stone_under_item_lock() {
  // Arven is a Supporter and can fetch Forest Seal Stone through Item lock; the Tool
  // then provides a VSTAR Power route rather than consuming Gladion: https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  const sim::Scenario scenario{"arven-fss-holds-gladion", sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{101};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Arven};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion should be held while Arven can retrieve a live Forest Seal Stone VSTAR route.");
  expect(sim::EngineTestAccess::play_arven(engine) &&
             contains(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone),
         "Arven should retrieve Forest Seal Stone through Item lock.");
}

void test_mysterious_treasure_holds_false_single_ultra_ball_payload_line() {
  // Mysterious Treasure pays its discard before it searches, and Ultra Ball must
  // later discard the fetched payload plus another card from hand:
  // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"mysterious-treasure-single-ultra-false-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{164};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::Gladion};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine, true),
         "Mysterious Treasure must be held when its only prospective Ultra Ball continuation lacks a second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::MysteriousTreasure) && contains(after.hand, sim::Card::UltraBall) &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::Gladion) &&
             after.discard.empty() && contains(after.deck, sim::Card::MegaDragonite),
         "Rejecting the false Mysterious Treasure continuation must preserve the exact state.");
}

void test_quick_ball_holds_false_single_ultra_ball_payload_line() {
  // Quick Ball pays its discard before it searches, and Ultra Ball must later
  // discard the fetched payload plus another card from hand:
  // https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"quick-ball-single-ultra-false-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{165};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::Gladion};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_quick_ball(engine, true),
         "Quick Ball must be held when its only prospective Ultra Ball continuation lacks a second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::QuickBall) && contains(after.hand, sim::Card::UltraBall) &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::Gladion) &&
             after.discard.empty() && contains(after.deck, sim::Card::DialgaGX),
         "Rejecting the false Quick Ball continuation must preserve the exact state.");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_requires_two_distinct_costs();
    test_evolution_incense_hands_payload_to_payable_ultra_ball();
    test_two_ultra_ball_payload_line_requires_second_cost();
    test_gladion_uses_prized_vstar_only_when_item_cost_is_unpayable();
    test_gladion_is_held_for_arven_forest_seal_stone_under_item_lock();
    test_mysterious_treasure_holds_false_single_ultra_ball_payload_line();
    test_quick_ball_holds_false_single_ultra_ball_payload_line();
    std::cout << "route reconciliation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
