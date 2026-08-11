#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static std::optional<Card> replaced_energy_cost(const Engine& engine) {
    return engine.quick_ball_crispin_replaced_energy_cost();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Crispin, sim::Card::Arven,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  return state;
}

void test_crispin_replaced_energy_uses_shared_jit_timing() {
  std::mt19937_64 rng(2757);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict{"issue-2757-strict", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  const sim::Scenario flex{"issue-2757-flex", sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, false, 5};
  const sim::Scenario control{"issue-2757-control", sim::DciProfile::NoDiscardControl,
                              sim::LockMode::None, false, 5};
  sim::Engine strict_engine(strict, recipe, rng);
  sim::Engine flex_engine(flex, recipe, rng);
  sim::Engine control_engine(control, recipe, rng);
  sim::EngineTestAccess::set_state(strict_engine, route_state());
  sim::EngineTestAccess::set_state(flex_engine, route_state());
  sim::EngineTestAccess::set_state(control_engine, route_state());

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Original route bug: https://github.com/FlareZ123/pokemon-sims/issues/1093
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2757
  expect(sim::EngineTestAccess::replaced_energy_cost(strict_engine) == sim::Card::Grass,
         "StrictJit lost the Crispin-replaced Quick Ball Grass cost.");
  expect(sim::EngineTestAccess::replaced_energy_cost(flex_engine) == sim::Card::Grass,
         "MatchupFlexJit still rejects the Crispin-replaced Quick Ball Grass cost.");
  expect(!sim::EngineTestAccess::replaced_energy_cost(control_engine).has_value(),
         "NoDiscardControl incorrectly entered the current-turn JIT replacement route.");
}
}  // namespace

int main() {
  try {
    test_crispin_replaced_energy_uses_shared_jit_timing();
    std::cout << "Issue 2757 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
