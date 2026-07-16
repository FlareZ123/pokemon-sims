#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool blender_energy_axis_can_finish_this_turn(const Engine& engine) {
    return engine.blender_energy_axis_can_finish_this_turn();
  }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"blender-energy-completion", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{737};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

sim::State state_with_energy(const int grass, const int fire) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, grass, fire,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

bool contains(const std::vector<sim::Card>& zone, const sim::Card card) {
  return std::find(zone.begin(), zone.end(), card) != zone.end();
}

void expect_blender_held(sim::State state, const std::string_view label) {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::blender_energy_axis_can_finish_this_turn(
          fixture.engine) ||
      sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": Blender must remain held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.deck, sim::Card::MegaDragonite) ||
      !after.discard.empty()) {
    throw std::runtime_error(std::string(label) +
                             ": the hold must preserve every zone.");
  }
}

void expect_blender_live(sim::State state, const std::string_view label) {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::blender_energy_axis_can_finish_this_turn(
          fixture.engine) ||
      !sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": Blender must remain a live route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error(std::string(label) +
                             ": the live route must discard Blender and payload.");
  }
}

void test_holds_after_manual_window_is_lost() {
  sim::State state = state_with_energy(1, 1);
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Grass);

  // A player may attach one Energy from hand during the turn. Once that action has
  // been used, the held Grass cannot complete Apex Dragon's GGF cost before the
  // strict-JIT payload expires:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_blender_held(std::move(state), "closed manual window");
}

void test_keeps_energy_complete_route() {
  expect_blender_live(state_with_energy(2, 1), "energy-complete VSTAR");
}

void test_keeps_unused_manual_attachment_route() {
  sim::State state = state_with_energy(1, 1);
  state.hand.push_back(sim::Card::Grass);

  // Vital Dance and Earthen Vessel put Basic Energy into hand. With the manual
  // attachment unused, that observable final Energy remains a legal same-turn route:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_blender_live(std::move(state), "unused manual attachment");
}

void test_keeps_crispin_one_energy_route_after_manual_use() {
  sim::State state = state_with_energy(1, 1);
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Crispin);

  // With both Basic Energy types searchable, Crispin attaches the needed Grass
  // directly, so the already-used manual attachment does not block completion:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_blender_live(std::move(state), "live one-Energy Crispin route");
}

void test_rejects_crispin_when_two_energy_remain_after_manual_use() {
  sim::State state = state_with_energy(1, 0);
  state.manual_energy_used = true;
  state.hand.push_back(sim::Card::Crispin);
  expect_blender_held(std::move(state),
                      "Crispin cannot attach both missing Energy");
}

void test_keeps_crispin_plus_manual_two_energy_route() {
  sim::State state = state_with_energy(1, 0);
  state.hand.push_back(sim::Card::Crispin);

  // Crispin attaches one searched type and puts the other into hand. The unused
  // manual attachment may then supply the second missing component:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  expect_blender_live(std::move(state), "Crispin plus manual attachment");
}

void test_non_strict_payload_banking_is_unchanged() {
  Fixture fixture;
  sim::Scenario scenario{"blender-energy-flex",
                         sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{738};
  sim::Engine engine{scenario, fixture.recipe, rng, &fixture.trace};
  sim::State state = state_with_energy(1, 1);
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  if (!sim::EngineTestAccess::blender_energy_axis_can_finish_this_turn(engine) ||
      !sim::EngineTestAccess::play_brilliant_blender(engine)) {
    throw std::runtime_error(
        "Non-strict payload banking must remain unchanged.");
  }
}

}  // namespace

int main() {
  test_holds_after_manual_window_is_lost();
  test_keeps_energy_complete_route();
  test_keeps_unused_manual_attachment_route();
  test_keeps_crispin_one_energy_route_after_manual_use();
  test_rejects_crispin_when_two_energy_remain_after_manual_use();
  test_keeps_crispin_plus_manual_two_energy_route();
  test_non_strict_payload_banking_is_unchanged();
  std::cout << "blender energy completion tests passed\n";
  return 0;
}
