#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
};

}  // namespace sim

namespace {
using sim::Card;
using sim::DciProfile;
using sim::Engine;
using sim::EngineTestAccess;
using sim::LockMode;
using sim::Pokemon;
using sim::Scenario;
using sim::State;
using sim::Tool;
using sim::TraceLog;

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

Scenario scenario() {
  return Scenario{"bench-vstar-energy-target", DciProfile::StrictJit, LockMode::None, false, 4};
}

State base_state() {
  State result;
  result.turn = 2;
  result.discard = {Card::MegaDragonite};
  result.discarded_this_turn = {Card::MegaDragonite};
  return result;
}

const Pokemon* find_benched_vstar(const State& state) {
  const auto it = std::find_if(state.bench.begin(), state.bench.end(), [](const Pokemon& pokemon) {
    return pokemon.card == Card::RegidragoVstar;
  });
  return it == state.bench.end() ? nullptr : &*it;
}

void test_manual_attach_powers_existing_vstar_route() {
  State state = base_state();
  state.active = Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 0, 0, Tool::None},
                 Pokemon{Card::LatiasEx, 1, 0, 0, Tool::None}};
  state.hand = {Card::Grass, Card::Fire};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{181};
  TraceLog trace{true, {}};
  Scenario fixture_scenario = scenario();
  Engine engine(fixture_scenario, recipe, rng, &trace);
  EngineTestAccess::set_state(engine, std::move(state));

  // Apex Dragon belongs to Regidrago VSTAR, and Latias ex can let the Basic Active retreat
  // so the Benched VSTAR becomes Active; the Energy should therefore land on that existing
  // VSTAR route, not on a different Basic Regidrago V. Sources:
  // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(EngineTestAccess::attach_manual(engine), "Manual attachment should be available.");
  const State& after = EngineTestAccess::state(engine);
  const Pokemon* vstar = find_benched_vstar(after);
  expect(vstar != nullptr, "The fixture should still contain the Benched VSTAR.");
  expect(after.active && after.active->card == Card::RegidragoV && after.active->grass == 1 && after.active->fire == 0,
         "The Active Basic Regidrago V should not receive the manual Energy.");
  expect(vstar->grass == 1 && vstar->fire == 0,
         "The manual Energy should target the existing Regidrago VSTAR route.");
}

void test_active_evolution_line_keeps_energy_target() {
  State state = base_state();
  state.active = Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 0, 0, Tool::None}};
  state.hand = {Card::RegidragoVstar, Card::Fire};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{182};
  TraceLog trace{true, {}};
  Scenario fixture_scenario = scenario();
  Engine engine(fixture_scenario, recipe, rng, &trace);
  EngineTestAccess::set_state(engine, std::move(state));

  // When the Active Regidrago V can legally evolve from hand this turn, powering it
  // remains a live Apex Dragon route. Evolution timing and one manual attachment are
  // core turn procedures: https://www.pokemon.com/us/pokemon-tcg/rules
  // Regidrago VSTAR's attack source is the searched/evolved card: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(EngineTestAccess::attach_manual(engine), "Manual attachment should be available.");
  const State& after = EngineTestAccess::state(engine);
  const Pokemon* vstar = find_benched_vstar(after);
  expect(vstar != nullptr, "The fixture should still contain the Benched VSTAR.");
  expect(after.active && after.active->card == Card::RegidragoV && after.active->grass == 1 && after.active->fire == 1,
         "The Active Regidrago V should keep the Energy target when it can evolve from hand.");
  expect(vstar->grass == 0 && vstar->fire == 0,
         "The existing Benched VSTAR should not steal Energy from a live Active evolution route.");
}

}  // namespace

int main() {
  try {
    test_manual_attach_powers_existing_vstar_route();
    test_active_evolution_line_keeps_energy_target();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
