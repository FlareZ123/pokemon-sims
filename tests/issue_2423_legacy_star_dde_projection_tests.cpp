#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static bool basic_completes(const Engine& engine, const Pokemon& pokemon, const Card energy) {
    return engine.legacy_star_basic_completes_apex(pokemon, energy);
  }
  static bool delayed_vessel(const Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
};
}  // namespace sim

namespace {
void expect(bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::Pokemon vstar(int grass, int fire, int dde) {
  sim::Pokemon pokemon{sim::Card::RegidragoVstar, 1, grass, fire, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

struct Fixture {
  sim::Scenario scenario{"issue-2423", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2423};
  sim::Engine engine{scenario, recipe, rng};
};

void test_completing_basic_truth_table() {
  Fixture f;
  // DDE-only is completed by either Basic. https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex GGF cost: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Bug: https://github.com/FlareZ123/pokemon-sims/issues/2423
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(0, 0, 1), sim::Card::Grass),
         "DDE-only + Grass should complete Apex");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(0, 0, 1), sim::Card::Fire),
         "DDE-only + Fire should complete Apex");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(1, 1, 0), sim::Card::Grass),
         "GF + Grass should complete Apex");
  expect(!sim::EngineTestAccess::basic_completes(f.engine, vstar(1, 1, 0), sim::Card::Fire),
         "GF + Fire should remain short");
  expect(sim::EngineTestAccess::basic_completes(f.engine, vstar(2, 0, 0), sim::Card::Fire),
         "GG + Fire should complete Apex");
  expect(!sim::EngineTestAccess::basic_completes(f.engine, vstar(2, 0, 0), sim::Card::Grass),
         "GG + Grass should remain short");
}

void test_delayed_vessel_accepts_dde_only() {
  Fixture f;
  sim::State state;
  state.turn = 2;
  state.active = vstar(0, 0, 1);
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(f.engine, std::move(state));
  expect(sim::EngineTestAccess::delayed_vessel(f.engine),
         "Delayed Vessel route missed DDE-only plus a searchable completing Basic");
}

void test_delayed_vessel_preserves_basic_controls() {
  Fixture f;
  sim::State state;
  state.turn = 2;
  state.active = vstar(1, 1, 0);
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(f.engine, std::move(state));
  expect(sim::EngineTestAccess::delayed_vessel(f.engine),
         "Delayed Vessel regressed canonical GF + Grass");
}
}  // namespace

int main() {
  try {
    test_completing_basic_truth_table();
    test_delayed_vessel_accepts_dde_only();
    test_delayed_vessel_preserves_basic_controls();
    std::cout << "Issue 2423 Legacy Star DDE projection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
