#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
};

}  // namespace sim

int main() {
  // Forest Seal Stone grants the attached Pokémon V access to Star Alchemy from
  // the Tool itself: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Path to the Peak removes Abilities from Rule Box Pokémon, rather than text
  // on attached Tools: https://api.pokemontcg.io/v2/cards/swsh6-148
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

  return 0;
}
