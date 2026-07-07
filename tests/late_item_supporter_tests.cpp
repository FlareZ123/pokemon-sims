#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};

}  // namespace sim

int main() {
  using namespace sim;

  const Scenario scenario{"late-item-supporter", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(43);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::MysteriousTreasure, Card::Dipplin};
  state.deck = {Card::MegaDragonite, Card::ProfessorBurnet, Card::TapuLeleGX};

  // Mysterious Treasure can find Tapu Lele-GX, Wonder Tag finds Professor Burnet, and Burnet can discard the current-turn payload: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/cel25c-60_A https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.supporter_used);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::ProfessorBurnet) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::MegaDragonite) == 1);
  assert(std::count(state.discarded_this_turn.begin(), state.discarded_this_turn.end(), Card::MegaDragonite) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);

  return 0;
}
