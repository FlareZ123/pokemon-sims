#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

int main() {
  using namespace sim;
  const Scenario scenario{"lusamine-arven-item-lock", DciProfile::StrictJit, LockMode::FullItem, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(146);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);

  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::Lusamine};
  state.discard = {Card::Arven, Card::Guzma};
  state.deck = {Card::RegidragoVstar, Card::ForestSealStone};

  // Lusamine can recover discarded Supporter cards: https://api.pokemontcg.io/v2/cards/sm4-96
  // Arven remains playable during Item lock and may choose Forest Seal Stone as its Tool target: https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Forest Seal Stone then grants Star Alchemy to the attached Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-156
  EngineTestAccess::run_turn(engine);

  assert(state.supporter_used);
  assert(std::count(state.hand.begin(), state.hand.end(), Card::Arven) == 1);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Lusamine) == 1);

  state.turn = 3;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.retreat_used = false;
  state.turn_ended = false;
  state.discarded_this_turn.clear();
  EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(EngineTestAccess::outcome(engine).used_fss);
  assert(std::count(state.discard.begin(), state.discard.end(), Card::Arven) == 1);
  return 0;
}
