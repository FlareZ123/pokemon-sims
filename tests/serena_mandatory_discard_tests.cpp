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

  const Scenario scenario{"serena-zero-discard", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(167);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Serena};
  state.deck = {Card::RegidragoVstar, Card::Grass, Card::Fire, Card::Dipplin, Card::QuickBall};

  // Serena permits zero through three discards before drawing until the player has five cards: https://api.pokemontcg.io/v2/cards/swsh12-164
  assert(EngineTestAccess::play_serena(engine));
  assert(state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Serena) == 0);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Serena) == 1);
  assert(state.discard.size() == 1);
  assert(state.hand.size() == 5U);
  assert(state.deck.empty());

  return 0;
}
