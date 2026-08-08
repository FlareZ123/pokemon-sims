#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool held_crispin_completion(const Engine& engine) {
    return engine.issue_1393_held_crispin_completion_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon active_regidrago(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoV, 1, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2445", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2445};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::Gladion, sim::Card::BrilliantBlender};
  return state;
}

void test_dde_plus_single_available_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(0, 0, 1);
  state.deck = {basic, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Crispin searches up to two Basic Energy of different types. A DDE-equipped
  // Dragon needs only one physical Basic Grass or Fire to pay Apex Dragon, so
  // the unavailable opposite Basic type cannot invalidate this current-turn line.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2445
  expect(sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "Crispin rejected a DDE route with one legal finishing Basic type.");
}

void test_basic_only_route_still_uses_two_physical_attachments() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(1, 0, 0);
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Without DDE, one existing Grass still requires Crispin plus the unused manual
  // attachment to supply the remaining Grass and Fire this turn.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // One manual Energy attachment each turn: https://www.pokemon.com/us/pokemon-tcg/rules
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "Basic-only Crispin plus manual-attachment control route regressed.");
}

void test_basic_only_route_rejects_missing_second_type() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(1, 0, 0);
  state.deck = {sim::Card::Grass, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // With no DDE, a second Grass alone cannot supply Apex Dragon's Fire requirement.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "Crispin accepted a Basic-only route with no Fire completion available.");
}

void test_dde_route_rejects_no_basic_energy() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(0, 0, 1);
  state.deck = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE supplies two units, while Apex Dragon still requires a third Energy unit.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "Crispin accepted a DDE route with no finishing Basic Energy in deck.");
}

}  // namespace

int main() {
  try {
    test_dde_plus_single_available_basic(sim::Card::Grass);
    test_dde_plus_single_available_basic(sim::Card::Fire);
    test_basic_only_route_still_uses_two_physical_attachments();
    test_basic_only_route_rejects_missing_second_type();
    test_dde_route_rejects_no_basic_energy();
    std::cout << "Issue 2445 Crispin DDE tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
