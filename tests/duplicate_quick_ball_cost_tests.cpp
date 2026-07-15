#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
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

void test_duplicate_quick_ball_is_legal_k1_cost() {
  const sim::Scenario scenario{"duplicate-quick-ball-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{74};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::QuickBall};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Quick Ball discards another card before searching a Basic Pokémon. The second
  // copy is a legal cost and the inspected deck contains a legal target:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  expect(sim::EngineTestAccess::play_quick_ball(engine, false),
         "Quick Ball should use the other Quick Ball as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Quick Ball should search the missing Basic Pokémon");
  expect(count(after.discard, sim::Card::QuickBall) == 2,
         "the played Quick Ball and its duplicate cost should both be discarded");
  expect(after.deck.empty(), "the searched Regidrago V should leave the deck");
}

void test_known_no_target_quick_ball_preserves_payload() {
  const sim::Scenario scenario{"quick-ball-known-no-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{553};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // K1 proves that no Basic Pokémon remains, so Quick Ball is known to have no
  // effect and its discard requirement cannot be paid:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_quick_ball(engine, true),
         "known zero-target Quick Ball should be held");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.hand == std::vector<sim::Card>{sim::Card::QuickBall,
                                               sim::Card::MegaDragonite},
         "Quick Ball and the payload should remain in hand");
  expect(after.deck == std::vector<sim::Card>{sim::Card::Dragapult},
         "the known Evolution-only deck should remain unchanged");
  expect(after.discard.empty(), "no play cost should enter discard");
}

void test_known_no_target_quick_ball_is_not_a_tate_route() {
  const sim::Scenario scenario{"tate-quick-known-no-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{554};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::TateLiza, sim::Card::QuickBall,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Tate planning must reject the same dead continuation because Quick Ball has no
  // legal Basic target in the inspected deck:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::has_live_one_discard_hand_payload_line(engine),
         "Tate should not preserve a route through known-dead Quick Ball");
}

}  // namespace

int main() {
  try {
    test_duplicate_quick_ball_is_legal_k1_cost();
    test_known_no_target_quick_ball_preserves_payload();
    test_known_no_target_quick_ball_is_not_a_tate_route();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
