#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"blender-energy-completion-gate",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{737};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State active_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect_rejected(sim::State state, const std::string_view label) {
  Fixture fixture;
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  const auto discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Blender must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve Blender and its payload.");
  }
}

void expect_allowed(sim::State state, const std::string_view label,
                    const sim::DciProfile dci = sim::DciProfile::StrictJit) {
  Fixture fixture;
  fixture.scenario.dci = dci;
  std::mt19937_64 rng{737};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::play_brilliant_blender(engine)) {
    throw std::runtime_error(std::string(label) + ": Blender route must remain live.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error(std::string(label) +
                             ": legal Blender route did not resolve normally.");
  }
}

void test_used_attachment_holds_blender() {
  sim::State state = active_vstar_state();
  state.manual_energy_used = true;

  // A player may attach one Energy from hand during a turn. Once that attachment
  // is spent, Blender cannot make this GF Regidrago VSTAR pay Apex Dragon's GGF
  // cost before a strict-JIT payload expires:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_rejected(std::move(state), "used manual attachment");
}

void test_complete_active_vstar_keeps_blender_live() {
  sim::State state = active_vstar_state();
  state.active->grass = 2;
  state.manual_energy_used = true;
  expect_allowed(std::move(state), "energy-complete Active VSTAR");
}

void test_held_final_energy_keeps_blender_live() {
  expect_allowed(active_vstar_state(), "held final Energy");
}

void test_payable_vessel_keeps_blender_live() {
  sim::State state = active_vstar_state();
  state.hand = {sim::Card::BrilliantBlender, sim::Card::EarthenVessel,
                sim::Card::Fire, sim::Card::Fire};

  // Earthen Vessel discards another card and searches up to two Basic Energy. The
  // final Grass can then use the still-open manual attachment:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_allowed(std::move(state), "payable Earthen Vessel");
}

void test_vessel_cannot_spend_blender_as_its_only_cost() {
  sim::State state = active_vstar_state();
  state.hand = {sim::Card::BrilliantBlender, sim::Card::EarthenVessel};
  expect_rejected(std::move(state), "Vessel with only Blender as cost");
}

void test_oricorio_keeps_blender_live() {
  sim::State state = active_vstar_state();
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Oricorio};

  // Vital Dance searches the final Basic Energy into hand, and the unused manual
  // attachment completes GGF:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_allowed(std::move(state), "held Oricorio Vital Dance");
}

void test_crispin_plus_manual_keeps_blender_live() {
  sim::State state = active_vstar_state();
  state.active->grass = 1;
  state.active->fire = 0;
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Crispin};

  // With both Energy types in deck, Crispin attaches Grass and puts Fire into hand;
  // the unused manual attachment then completes GF:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_allowed(std::move(state), "Crispin plus manual attachment");
}

void test_crispin_direct_attach_survives_used_manual() {
  sim::State state = active_vstar_state();
  state.manual_energy_used = true;
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Crispin};
  expect_allowed(std::move(state), "Crispin direct final attachment");
}

void test_crispin_cannot_finish_two_same_type_without_held_copy() {
  sim::State state = active_vstar_state();
  state.active->grass = 0;
  state.active->fire = 1;
  state.hand = {sim::Card::BrilliantBlender, sim::Card::Crispin};
  expect_rejected(std::move(state), "Crispin cannot supply two Grass");
}

void test_no_discard_control_banking_is_unchanged() {
  sim::State state = active_vstar_state();
  state.manual_energy_used = true;
  expect_allowed(std::move(state), "no-discard-control banking",
                 sim::DciProfile::NoDiscardControl);
}

}  // namespace

int main() {
  test_used_attachment_holds_blender();
  test_complete_active_vstar_keeps_blender_live();
  test_held_final_energy_keeps_blender_live();
  test_payable_vessel_keeps_blender_live();
  test_vessel_cannot_spend_blender_as_its_only_cost();
  test_oricorio_keeps_blender_live();
  test_crispin_plus_manual_keeps_blender_live();
  test_crispin_direct_attach_survives_used_manual();
  test_crispin_cannot_finish_two_same_type_without_held_copy();
  test_no_discard_control_banking_is_unchanged();
  std::cout << "All Brilliant Blender Energy-completion gate tests passed.\n";
  return 0;
}
