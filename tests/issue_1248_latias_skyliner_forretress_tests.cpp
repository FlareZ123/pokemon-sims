#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool use_exploding_energy_for_setup(Engine& engine) {
    return engine.use_exploding_energy_for_setup();
  }
};

}  // namespace sim

namespace {

sim::DeckRecipe forretress_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.emplace_back(sim::Card::Pineco, 1);
  recipe.emplace_back(sim::Card::ForretressEx, 1);
  return recipe;
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {
      sim::Pokemon{sim::Card::ForretressEx, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1},
  };
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  return state;
}

void test_skyliner_preserves_all_exploding_energy_grass() {
  const sim::Scenario scenario{"issue-1248-skyliner", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1248};
  const sim::DeckRecipe recipe = forretress_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  // Latias ex sv8-76 is a Basic Pokémon with printed Retreat Cost 2, while
  // Skyliner gives the player's Basic Pokémon in play no Retreat Cost. Exploding
  // Energy may therefore attach all three Grass to Regidrago and retreat for free:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::use_exploding_energy_for_setup(engine)) {
    throw std::runtime_error("The unlocked Latias Exploding Energy route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 3 || after.active->fire != 1 ||
      count(after.discard, sim::Card::Grass) != 0 || !after.retreat_used) {
    throw std::runtime_error("Skyliner did not preserve all three searched Grass during the free retreat.");
  }
}

void test_rule_box_ability_lock_blocks_forretress_route() {
  const sim::Scenario scenario{"issue-1248-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  std::mt19937_64 rng{12481};
  const sim::DeckRecipe recipe = forretress_recipe();
  sim::Engine engine(scenario, recipe, rng);
  const sim::State before = exact_state();
  sim::EngineTestAccess::set_state(engine, before);

  // The modeled Rule Box Ability lock disables both Forretress ex's Exploding
  // Energy and Latias ex's Skyliner, so no partial attachment or retreat may occur:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  if (sim::EngineTestAccess::use_exploding_energy_for_setup(engine)) {
    throw std::runtime_error("Rule Box Ability lock allowed Exploding Energy to resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::LatiasEx ||
      after.deck != before.deck || !after.discard.empty() || after.retreat_used) {
    throw std::runtime_error("The locked negative control changed the game state.");
  }
}

}  // namespace

int main() {
  try {
    test_skyliner_preserves_all_exploding_energy_grass();
    test_rule_box_ability_lock_blocks_forretress_route();
    std::cout << "Issue 1248 Latias Skyliner tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
