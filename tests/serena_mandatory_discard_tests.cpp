#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_serena(Engine& engine) { return engine.play_serena(); }
};

}  // namespace sim

int main() {
  using namespace sim;

  const Scenario scenario{"serena-mandatory-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(164);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena};
  state.deck = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::QuickBall};

  // Serena's draw mode says “Discard up to 3 cards” and then requires at least one discard before drawing: https://api.pokemontcg.io/v2/cards/swsh12-164
  assert(!EngineTestAccess::play_serena(engine));
  assert(!state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 1);
  assert(state.discard.empty());

  return 0;
}
