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

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_policy_unavailable_quick_ball() {
  // Quick Ball requires discarding another card before its Basic-Pokémon search:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Strict-JIT policy preserves the pending Gladion Prize-recovery connector:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-policy-unavailable-quick-ball", sim::DciProfile::StrictJit,
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
    throw std::runtime_error("Gladion must be played when Quick Ball has no policy-admissible discard cost");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoV) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the prized Regidrago V");
  }
}

void test_item_locked_forest_seal_stone() {
  // Forest Seal Stone is a Pokémon Tool and must be attached before its VSTAR Power can be used:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // Gladion may exchange itself with a card from the Prize cards:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-item-locked-fss", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(58);
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::Gladion, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, state);

  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion must not be held for a Forest Seal Stone blocked by Item lock");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoVstar) || !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must exchange with the prized Regidrago VSTAR");
  }
}

}  // namespace

int main() {
  test_policy_unavailable_quick_ball();
  test_item_locked_forest_seal_stone();
  return 0;
}
