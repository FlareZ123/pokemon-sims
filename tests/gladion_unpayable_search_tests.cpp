#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

int main() {
  // Quick Ball requires discarding another card before its Basic-Pokémon search:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-unpayable-quick-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(49);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::QuickBall};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must be played when Quick Ball has no legal discard cost");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (std::find(after.hand.begin(), after.hand.end(), sim::Card::RegidragoV) == after.hand.end()) {
    throw std::runtime_error("Gladion must recover the prized Regidrago V");
  }
  if (std::find(after.prizes.begin(), after.prizes.end(), sim::Card::Gladion) == after.prizes.end()) {
    throw std::runtime_error("Gladion must replace the selected Prize card");
  }
  return 0;
}
