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
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_quick_ball_can_discard_a_distinct_ultra_ball() {
  // Quick Ball discards another card from hand before it searches for a Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago V is a Basic Pokémon and is therefore a legal Quick Ball target:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  sim::Scenario scenario{"quick-ball-ultra-ball-cost", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::QuickBall, sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoV};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{137};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball should discard the distinct Ultra Ball and search Regidrago V.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.hand, sim::Card::RegidragoV),
         "Quick Ball should put the Basic Regidrago V into hand.");
  expect(contains(result.discard, sim::Card::QuickBall) && contains(result.discard, sim::Card::UltraBall),
         "Quick Ball and its distinct Ultra Ball discard cost should both enter discard.");
}

}  // namespace

int main() {
  try {
    test_quick_ball_can_discard_a_distinct_ultra_ball();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
