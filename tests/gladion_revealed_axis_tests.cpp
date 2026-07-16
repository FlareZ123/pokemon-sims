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

sim::State state_for(const bool basic_in_play) {
  sim::State state;
  state.turn = 2;
  state.active = basic_in_play ? sim::Pokemon{sim::Card::RegidragoV, 0}
                               : sim::Pokemon{sim::Card::Oricorio, 0};
  state.hand = basic_in_play ? std::vector<sim::Card>{sim::Card::Gladion}
                             : std::vector<sim::Card>{sim::Card::Gladion, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::RegidragoV,
                  sim::Card::Dipplin, sim::Card::Powerglass,
                  sim::Card::FieldBlower, sim::Card::ChaoticSwell};
  return state;
}

void missing_basic_beats_redundant_vstar() {
  const sim::Scenario scenario{"issue-780-basic", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{780};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, state_for(false));
  // Gladion chooses after the Prize reveal. Regidrago V is the exact missing axis:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/780
  if (!sim::EngineTestAccess::play_gladion(engine)) throw std::runtime_error("Gladion did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoV) ||
      std::count(after.hand.begin(), after.hand.end(), sim::Card::RegidragoVstar) != 1 ||
      !contains(after.prizes, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Gladion selected a redundant VSTAR before the missing Basic.");
  }
}

void missing_vstar_stays_priority() {
  const sim::Scenario scenario{"issue-780-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7801};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, state_for(true));
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/780
  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Gladion failed to recover the genuinely missing VSTAR.");
  }
}
}  // namespace

int main() {
  missing_basic_beats_redundant_vstar();
  missing_vstar_stays_priority();
  return 0;
}
