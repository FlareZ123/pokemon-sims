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

  // When Crispin finds only one Basic Energy, the official ruling puts it in hand
  // and forbids attaching it through Crispin. The still-unused T2 manual attachment
  // can attach that sole Grass or Fire to the DDE-equipped Regidrago, completing Apex.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2445
  expect(sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "Crispin-to-hand plus manual attachment rejected a DDE finishing Basic.");
}

void test_single_basic_requires_unused_manual_attachment() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(0, 0, 1);
  state.manual_energy_used = true;
  state.deck = {sim::Card::Grass, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One-card Crispin cannot attach the found Energy itself, so once the manual
  // attachment is already spent there is no current-turn DDE + Basic completion.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Manual Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
  expect(!sim::EngineTestAccess::held_crispin_completion(fixture.engine),
         "One-card Crispin route ignored that the manual attachment was spent.");
}

void test_basic_only_route_still_uses_two_physical_attachments() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = active_regidrago(1, 0, 0);
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Without DDE, one existing Grass still requires Crispin to attach one searched
  // Basic and the unused manual attachment to supply the other searched Basic.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
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

  // With no DDE, the sole searched Grass goes to hand and a manual Grass cannot
  // satisfy Apex Dragon's Fire requirement.
  // Crispin ruling, Stellar Crown FAQ: https://compendium.pokegym.net/category/5-trainers/crispin/
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
    test_single_basic_requires_unused_manual_attachment();
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
