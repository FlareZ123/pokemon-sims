#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
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

}  // namespace

int main() {
  try {
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

    expect(sim::EngineTestAccess::play_quick_ball(engine, false),
           "Quick Ball should use the other Quick Ball as its discard cost");

    const sim::State& after = sim::EngineTestAccess::state(engine);
    expect(contains(after.hand, sim::Card::RegidragoV),
           "Quick Ball should search the missing Basic Pokemon");
    expect(count(after.discard, sim::Card::QuickBall) == 2,
           "the played Quick Ball and its duplicate cost should both be discarded");
    expect(after.deck.empty(), "the searched Regidrago V should leave the deck");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
