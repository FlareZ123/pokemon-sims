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

void test_legacy_star_recovers_evolution_incense_payload_bridge() {
  using namespace sim;
  const Scenario scenario{"legacy-star-evolution-incense-payload", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(168);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.deck = {Card::MegaDragonite, Card::Grass, Card::Fire, Card::Crispin,
                Card::Arven, Card::Dipplin, Card::EvolutionIncense, Card::MysteriousTreasure};

  // Legacy Star can return up to two discarded cards. Evolution Incense can then
  // fetch an Evolution Dragon payload, and Mysterious Treasure can discard that
  // fetched Dragon from hand as its cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  EngineTestAccess::run_turn(engine);

  assert(contains(state.discard, Card::EvolutionIncense));
  assert(contains(state.discard, Card::MysteriousTreasure));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
  assert(EngineTestAccess::payload_ready(engine));
}

}  // namespace

int main() {
  test_legacy_star_recovers_evolution_incense_payload_bridge();
  return 0;
}
