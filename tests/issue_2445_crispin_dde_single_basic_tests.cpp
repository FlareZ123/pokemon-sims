#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool held_crispin(const Engine& engine) {
    return engine.issue_1393_held_crispin_completion_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::Pokemon prior_turn_regi(const int grass, const int fire, const int dde) {
  sim::Pokemon pokemon{sim::Card::RegidragoV, 1, grass, fire, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

struct Fixture {
  sim::Scenario scenario{"issue-2445", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2445};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state(const sim::Pokemon& active) {
  sim::State state;
  state.turn = 2;
  state.active = active;
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::Gladion, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  return state;
}

void test_dde_grass_only() {
  Fixture fixture;
  auto state = base_state(prior_turn_regi(0, 0, 1));
  state.deck.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  // With one found Basic, Crispin puts it into hand, then the unused manual
  // attachment supplies it. https://compendium.pokegym.net/category/5-trainers/crispin/
  // DDE + Grass pays Apex. https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2445
  expect(sim::EngineTestAccess::held_crispin(fixture.engine),
         "DDE plus Grass-only Crispin route was rejected");
}

void test_dde_fire_only() {
  Fixture fixture;
  auto state = base_state(prior_turn_regi(0, 0, 1));
  state.deck.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::held_crispin(fixture.engine),
         "DDE plus Fire-only Crispin route was rejected");
}

void test_basic_only_controls() {
  Fixture grass_fixture;
  auto grass_state = base_state(prior_turn_regi(1, 1, 0));
  grass_state.deck.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(grass_fixture.engine, std::move(grass_state));
  expect(sim::EngineTestAccess::held_crispin(grass_fixture.engine),
         "GF plus Grass control regressed");

  Fixture fire_fixture;
  auto fire_state = base_state(prior_turn_regi(2, 0, 0));
  fire_state.deck.push_back(sim::Card::Fire);
  sim::EngineTestAccess::set_state(fire_fixture.engine, std::move(fire_state));
  expect(sim::EngineTestAccess::held_crispin(fire_fixture.engine),
         "GG plus Fire control regressed");
}
}  // namespace

int main() {
  try {
    test_dde_grass_only();
    test_dde_fire_only();
    test_basic_only_controls();
    std::cout << "Issue 2445 Crispin DDE single-Basic tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
