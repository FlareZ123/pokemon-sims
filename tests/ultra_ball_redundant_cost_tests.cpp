#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_ultra_ball(Engine& engine) { return engine.play_ultra_ball(false); }
};

}  // namespace sim

int main() {
  using namespace sim;
  const Scenario scenario{"ultra-ball-redundant-cost", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(151);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1};
  state.hand = {Card::UltraBall, Card::UltraBall, Card::Dipplin};
  state.deck = {Card::RegidragoVstar};

  // Ultra Ball discards 2 other cards from hand before it searches for a Pokémon: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Regidrago VSTAR is a Pokémon and is a legal Ultra Ball target: https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_ultra_ball(engine));
  assert(std::count(state.hand.begin(), state.hand.end(), Card::RegidragoVstar) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::UltraBall) == 2);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Dipplin) == 1);
  return 0;
}
