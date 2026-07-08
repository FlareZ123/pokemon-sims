#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_tate_switch_preserves_blender_payload_line() {
  using namespace sim;
  const Scenario scenario{"tate-switch-blender-payload", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(164);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 0, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::TateLiza, Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};

  // Tate & Liza can switch the Active with a Benched Pokémon, and Brilliant Blender
  // can then search and discard the current-turn Dragon payload as a later Item play:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(contains(state.discard, Card::TateLiza));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

}  // namespace

int main() {
  test_tate_switch_preserves_blender_payload_line();
  return 0;
}
