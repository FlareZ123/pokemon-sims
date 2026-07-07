#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_ultra_ball(Engine& engine, bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State state_with_hand(const std::vector<sim::Card>& hand) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = hand;
  state.deck = {sim::Card::RegidragoV};
  return state;
}

void test_ultra_ball_requires_two_distinct_hand_cards() {
  // Ultra Ball requires discarding 2 other cards from hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  sim::Scenario scenario{"ultra-ball-singleton-cost", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{51};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, state_with_hand({sim::Card::UltraBall, sim::Card::Dipplin}));

  expect(!sim::EngineTestAccess::play_ultra_ball(engine, false),
         "Ultra Ball must be held when only one other hand card can pay its two-card cost");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.hand, sim::Card::UltraBall) == 1 && count(after.hand, sim::Card::Dipplin) == 1,
         "failed Ultra Ball selection must restore the original hand");
  expect(after.discard.empty(), "failed Ultra Ball selection must not manufacture or discard a second singleton");
  expect(count(after.deck, sim::Card::RegidragoV) == 1, "failed Ultra Ball selection must not search the deck");
}

void test_ultra_ball_uses_a_real_second_discard_cost() {
  // The second cost may be a different legal card after the first is removed: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  sim::Scenario scenario{"ultra-ball-two-costs", sim::DciProfile::MatchupFlexJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{52};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, state_with_hand({sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::MawileGX}));

  expect(sim::EngineTestAccess::play_ultra_ball(engine, false),
         "Ultra Ball should resolve when two different legal costs are in hand");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::UltraBall) == 1 && count(after.discard, sim::Card::Dipplin) == 1 &&
             count(after.discard, sim::Card::MawileGX) == 1,
         "Ultra Ball must discard itself and two real, distinct cards");
  expect(count(after.hand, sim::Card::RegidragoV) == 1,
         "Ultra Ball should still search the requested Basic Pokémon after a legal cost payment");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_requires_two_distinct_hand_cards();
    test_ultra_ball_uses_a_real_second_discard_cost();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
