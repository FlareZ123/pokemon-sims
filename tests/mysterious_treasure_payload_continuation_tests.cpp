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
  static bool play_mysterious_treasure(Engine& engine) { return engine.play_mysterious_treasure(true); }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(true); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool can_play_payload_this_turn(const Engine& engine) {
    return engine.can_play_payload_this_turn();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_two_mysterious_treasures_do_not_strand_a_fetched_payload() {
  // Mysterious Treasure must discard a card before it searches for a Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // Apex Dragon needs a Dragon Pokémon attack in discard, so a fetched payload must
  // still have a legal same-turn discard continuation: https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::Scenario scenario{"two-treasure-payload-continuation", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  // The Supporter play has already been used, so Tapu Lele-GX cannot open a
  // separate Supporter-search axis in this payload-only fixture:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  state.supporter_used = true;

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{140};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine),
         "The second Mysterious Treasure is the only current cost and cannot also be the payload outlet.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.hand.size() == 2U && contains(result.hand, sim::Card::MysteriousTreasure),
         "The policy should retain both Mysterious Treasures rather than strand a fetched payload.");
  expect(contains(result.deck, sim::Card::MegaDragonite),
         "The Dragon payload should remain in deck until a live discard continuation exists.");
}

void test_mysterious_treasure_uses_ultra_ball_cost_and_preserves_quick_ball_payload_outlet() {
  const sim::Scenario scenario{"treasure-ultra-cost-quick-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::UltraBall, sim::Card::QuickBall};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  state.supporter_used = true;

  // Mysterious Treasure may discard the distinct held Ultra Ball, search the
  // Dragon payload, and leave Quick Ball to discard that fetched payload before
  // searching the Basic Regidrago V:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{193};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(sim::EngineTestAccess::play_mysterious_treasure(engine),
         "Mysterious Treasure should discard held Ultra Ball and fetch the Dragon payload.");
  expect(contains(sim::EngineTestAccess::state(engine).discard, sim::Card::UltraBall),
         "The held Ultra Ball should pay Mysterious Treasure's one-card cost.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite),
         "Mysterious Treasure should search Mega Dragonite ex into hand.");

  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should remain available to discard the fetched Dragon payload.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::MegaDragonite) &&
             contains(result.discarded_this_turn, sim::Card::MegaDragonite),
         "Quick Ball should place Mega Dragonite ex in discard during the current turn.");
  expect(contains(result.hand, sim::Card::RegidragoV),
         "Quick Ball should complete its required Basic Pokémon search.");
  // Apex Dragon uses an attack from a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The Mysterious Treasure into Quick Ball route should satisfy strict-JIT payload timing.");
}

void test_strict_jit_holds_sole_known_payload_when_no_vstar_can_exist_this_turn() {
  const sim::Scenario scenario{"strict-jit-sole-known-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{229};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV};
  state.prizes = {sim::Card::Dragapult, sim::Card::GoodraVstar, sim::Card::DialgaGX,
                  sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // K1 proves Mega Dragonite ex is the sole remaining modeled payload. Quick Ball
  // could Bench Regidrago V, but that new Basic cannot evolve in the same turn, so
  // strict JIT preserves the only future Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(!sim::EngineTestAccess::can_play_payload_this_turn(engine),
         "Strict JIT should protect the sole known payload when no VSTAR can exist this turn.");
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should not spend the sole known payload for a newly played Regidrago V.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::MegaDragonite) && contains(after.deck, sim::Card::RegidragoV),
         "The sole payload and Basic target should remain in their original zones.");
}

void test_no_control_still_allows_early_payload_banking() {
  const sim::Scenario scenario{"no-control-new-regidrago", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{230};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The no-discard-control profile intentionally permits prior-turn payload
  // banking through Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  expect(sim::EngineTestAccess::can_play_payload_this_turn(engine),
         "No-control should keep its early payload-banking policy.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should discard the payload and find Regidrago V in no-control.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.hand, sim::Card::RegidragoV),
         "No-control should preserve the documented early payload line.");
}

void test_prior_turn_regidrago_opens_strict_jit_window() {
  const sim::Scenario scenario{"strict-jit-evolvable-regidrago", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{231};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A Regidrago V that entered play on turn 1 may evolve on turn 2, so the same-turn
  // strict-JIT payload window is live: https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::can_play_payload_this_turn(engine),
         "A prior-turn Regidrago V should keep the strict-JIT payload window open.");
}

}  // namespace

int main() {
  try {
    test_two_mysterious_treasures_do_not_strand_a_fetched_payload();
    test_mysterious_treasure_uses_ultra_ball_cost_and_preserves_quick_ball_payload_outlet();
    test_strict_jit_holds_sole_known_payload_when_no_vstar_can_exist_this_turn();
    test_no_control_still_allows_early_payload_banking();
    test_prior_turn_regidrago_opens_strict_jit_window();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
