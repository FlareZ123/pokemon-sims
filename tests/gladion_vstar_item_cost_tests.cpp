#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
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

sim::State prized_vstar_state(std::vector<sim::Card> hand) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = std::move(hand);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  return state;
}

void test_unpayable_mysterious_treasure_recovers_vstar() {
  // Mysterious Treasure needs a hand discard before its Dragon search:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // Gladion exchanges itself with a Prize card: https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-unpayable-mysterious-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(116);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure}));

  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Gladion must recover the prized VSTAR when Mysterious Treasure has no discard cost.");
  }
}

void test_unpayable_ultra_ball_recovers_vstar() {
  // Ultra Ball needs 2 other hand cards before its Pokémon search:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Gladion exchanges itself with a Prize card: https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-unpayable-ultra-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(117);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, prized_vstar_state({sim::Card::Gladion, sim::Card::UltraBall, sim::Card::Dipplin}));

  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Gladion must recover the prized VSTAR when Ultra Ball has only one discard cost.");
  }
}

void test_payable_mysterious_treasure_preserves_gladion() {
  // Mysterious Treasure may search a Dragon Pokémon once its discard cost is paid:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  const sim::Scenario scenario{"gladion-payable-mysterious-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(118);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure, sim::Card::Dipplin}));

  if (sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion must remain available when Mysterious Treasure has a legal discard cost.");
  }
}

}  // namespace

int main() {
  try {
    test_unpayable_mysterious_treasure_recovers_vstar();
    test_unpayable_ultra_ball_recovers_vstar();
    test_payable_mysterious_treasure_preserves_gladion();
    std::cout << "Gladion VSTAR Item-cost tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
