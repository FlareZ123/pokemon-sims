#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void run_full_turn(Engine& engine) {
    // Full-turn fixtures mirror Engine::run(): draw before tactical policy, then stop
    // when the mandatory draw ends the game:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
    const int turn = engine.state_.turn;
    engine.begin_turn(turn);
    if (!engine.state_.turn_ended) engine.run_turn();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_full_turn_draw_precedes_tactical_evolution() {
  using namespace sim;
  const Scenario scenario{"full-turn-draw-before-policy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(489);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.deck = {Card::RegidragoVstar};

  // The player draws before taking tactical actions. Drawing the VSTAR from
  // deck.back() makes the subsequent legal evolution possible in the same full turn:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  EngineTestAccess::run_full_turn(engine);

  assert(!state.turn_ended);
  assert(state.deck.empty());
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(!contains(state.hand, Card::RegidragoVstar));
}

void test_empty_deck_draw_prevents_tactical_evolution() {
  using namespace sim;
  const Scenario scenario{"empty-deck-stops-before-policy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(490);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::None};
  state.hand = {Card::RegidragoVstar};
  state.deck.clear();

  // A player who cannot draw at the start of the turn loses before taking further
  // actions. The held VSTAR remains in hand and the Active does not evolve:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L20-L34
  EngineTestAccess::run_full_turn(engine);

  assert(state.turn_ended);
  assert(state.active && state.active->card == Card::RegidragoV);
  assert(contains(state.hand, Card::RegidragoVstar));
}

}  // namespace

int main() {
  test_full_turn_draw_precedes_tactical_evolution();
  test_empty_deck_draw_prevents_tactical_evolution();
  return 0;
}
