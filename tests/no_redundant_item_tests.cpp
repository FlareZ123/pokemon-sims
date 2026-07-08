#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void play_items_until_stable(Engine& engine, const bool permit_payload) {
    engine.play_items_until_stable(permit_payload);
  }
};

}  // namespace sim

namespace {

void test_ready_state_holds_hisuian_heavy_ball() {
  using namespace sim;
  const Scenario scenario{"ready-state-holds-heavy-ball", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(147);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::HisuianHeavyBall};
  state.prizes = {Card::RegidragoV, Card::Grass, Card::Fire, Card::Dipplin, Card::MawileGX, Card::Guzma};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Heavy Ball's Prize reveal/exchange is optional, so a ready setup policy keeps
  // the Item instead of spending it without an unresolved axis: https://api.pokemontcg.io/v2/cards/swsh10-146
  EngineTestAccess::play_items_until_stable(engine, true);

  assert(std::count(state.hand.begin(), state.hand.end(), Card::HisuianHeavyBall) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::HisuianHeavyBall) == 0);
  assert(std::count(state.prizes.begin(), state.prizes.end(), Card::RegidragoV) == 1);
}

}  // namespace

int main() {
  test_ready_state_holds_hisuian_heavy_ball();
  return 0;
}
