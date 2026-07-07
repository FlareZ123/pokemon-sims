#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_oricorio_if_useful(Engine& engine) { return engine.bench_oricorio_if_useful(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_off_type_fire_does_not_hide_missing_grass_connector() {
  // Regidrago V needs two Grass and one Fire Energy: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Vital Dance can search the remaining Basic Grass Energy: https://api.pokemontcg.io/v2/cards/sm2-55
  sim::Scenario scenario{"oricorio-type-sensitive-energy", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Fire};
  state.deck = {sim::Card::Grass};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{107};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_oricorio_if_useful(engine),
         "An off-type Fire Energy must not suppress Vital Dance for a missing Grass Energy.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Grass),
         "Vital Dance should put the missing Grass Energy into hand.");
}

}  // namespace

int main() {
  try {
    test_off_type_fire_does_not_hide_missing_grass_connector();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
