#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
};

}  // namespace sim

namespace {

void test_star_alchemy_through_rule_box_lock() {
  // Forest Seal Stone grants the attached Pokémon V access to Star Alchemy from
  // the Tool itself: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Path to the Peak does not stop that Tool-granted access:
  // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  const sim::Scenario scenario{"fss-under-rule-box-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  std::mt19937_64 rng{77};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0, sim::Tool::ForestSealStone};
  state.deck = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::use_fss(engine)) {
    throw std::runtime_error("Star Alchemy should remain usable through a Rule Box Ability lock.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.vstar_power_used ||
      std::find(after.hand.begin(), after.hand.end(), sim::Card::RegidragoVstar) == after.hand.end()) {
    throw std::runtime_error("Star Alchemy should consume the VSTAR Power and search Regidrago VSTAR.");
  }
}

void test_attach_forest_seal_stone_to_regidrago_vstar() {
  // Pokémon V includes Pokémon VSTAR, and Forest Seal Stone may be attached to a
  // Pokémon V: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  const sim::Scenario scenario{"fss-attach-to-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{78};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::attach_fss(engine) ||
      sim::EngineTestAccess::state(engine).active->tool != sim::Tool::ForestSealStone) {
    throw std::runtime_error("Forest Seal Stone should attach directly to Regidrago VSTAR.");
  }
}

void test_use_forest_seal_stone_after_evolution_to_vstar() {
  // TPCi explicitly permits a Seal Stone VSTAR Power line even when its Pokémon V
  // evolves into a Pokémon VSTAR later: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Pokémon VSTAR remains a Pokémon V: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  const sim::Scenario scenario{"fss-use-after-vstar-evolution", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{79};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::ForestSealStone};
  state.discard = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::use_fss(engine)) {
    throw std::runtime_error("Regidrago VSTAR should retain access to attached Forest Seal Stone.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.vstar_power_used ||
      std::find(after.hand.begin(), after.hand.end(), sim::Card::Fire) == after.hand.end()) {
    throw std::runtime_error("Star Alchemy should search the missing Fire Energy after evolution.");
  }
}

void test_arven_finds_forest_seal_stone_for_regidrago_vstar() {
  // Arven may search a Pokémon Tool, Forest Seal Stone attaches to Pokémon V, and
  // Pokémon V includes Pokémon VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  const sim::Scenario scenario{"arven-fss-for-vstar", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{80};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.discard = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::play_arven(engine)) {
    throw std::runtime_error("Arven should recognize the open Regidrago VSTAR Tool slot.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (std::find(after.hand.begin(), after.hand.end(), sim::Card::ForestSealStone) == after.hand.end()) {
    throw std::runtime_error("Arven should search Forest Seal Stone for Regidrago VSTAR.");
  }
}

}  // namespace

int main() {
  test_star_alchemy_through_rule_box_lock();
  test_attach_forest_seal_stone_to_regidrago_vstar();
  test_use_forest_seal_stone_after_evolution_to_vstar();
  test_arven_finds_forest_seal_stone_for_regidrago_vstar();
  return 0;
}
