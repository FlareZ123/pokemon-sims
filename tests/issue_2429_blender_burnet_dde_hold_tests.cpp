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
  static bool hold_blender(const Engine& engine) {
    return engine.issue_1646_hold_blender_for_burnet_finish_visible();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon attacker(const int grass, const int fire, const int dde) {
  sim::Pokemon result{sim::Card::RegidragoVstar, 1, grass, fire,
                      sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2429", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2429};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Dragapult};
  state.discard = {sim::Card::EarthenVessel, sim::Card::QuickBall};
  state.discarded_this_turn = {sim::Card::QuickBall};
  return state;
}

void test_dde_complete_is_held(const sim::Card basic) {
  Fixture fixture;
  sim::State state = base_state();
  state.manual_energy_used = true;
  state.active = attacker(basic == sim::Card::Grass ? 1 : 0,
                          basic == sim::Card::Fire ? 1 : 0, 1);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // One DDE plus either Basic pays Apex, so Burnet can supply the missing payload
  // without spending the one-copy Blender.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2429
  expect(sim::EngineTestAccess::hold_blender(fixture.engine),
         "DDE-complete Active failed to hold Blender for Burnet.");
}

void test_original_pre_attachment_control() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = attacker(1, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The original GF state remains a hold because the unused manual Grass attachment
  // completes Apex before Burnet supplies payload.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original contract: https://github.com/FlareZ123/pokemon-sims/issues/1646
  expect(sim::EngineTestAccess::hold_blender(fixture.engine),
         "Original GF plus held Grass Blender-hold route regressed.");
}

void test_nonfinishing_basic_rejected() {
  Fixture fixture;
  sim::State state = base_state();
  state.active = attacker(0, 1, 0);
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(!sim::EngineTestAccess::hold_blender(fixture.engine),
         "Blender was held when one Grass still does not pay Apex.");
}

}  // namespace

int main() {
  try {
    test_dde_complete_is_held(sim::Card::Grass);
    test_dde_complete_is_held(sim::Card::Fire);
    test_original_pre_attachment_control();
    test_nonfinishing_basic_rejected();
    std::cout << "Issue 2429 Blender/Burnet DDE hold tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
