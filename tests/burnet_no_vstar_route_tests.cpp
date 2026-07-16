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
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"burnet-no-vstar-route", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{797};
  sim::Engine engine{scenario, recipe, rng};
};

void expect_rejected(sim::State state, const std::string_view label) {
  Fixture fixture;
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  const auto discard_before = state.discard;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error(std::string(label) + ": Burnet must be held.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      after.discard != discard_before || after.supporter_used) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve every resource.");
  }
}

void test_strict_jit_holds_burnet_without_vstar_route() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Dipplin,
                sim::Card::Fire, sim::Card::RoseannesBackup};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  state.prizes = {sim::Card::RegidragoVstar};

  // A prior-turn Regidrago V keeps the shared payload window open, but a known
  // prized VSTAR and no held evolution-search Item leave no same-turn Apex Dragon
  // route. Burnet must preserve its Supporter slot and Dragon payloads:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/797
  expect_rejected(std::move(state), "no VSTAR route");
}

void test_held_vstar_preserves_strict_jit_route() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::RegidragoVstar};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The held VSTAR legally evolves the prior-turn Active Regidrago V after
  // Burnet, so the searched payload belongs to a same-turn GGF route:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/797
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("A held VSTAR must preserve the Burnet route.");
  }
}

void test_payable_treasure_preserves_strict_jit_route() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MysteriousTreasure,
                sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Mysterious Treasure has a legal one-card cost and can find Regidrago VSTAR,
  // preserving the same-turn evolution and payload route:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/797
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("A payable Treasure must preserve the Burnet route.");
  }
}

void test_no_discard_control_keeps_no_vstar_banking() {
  sim::Scenario scenario{"burnet-no-vstar-banking",
                         sim::DciProfile::NoDiscardControl,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{798};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 1, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // No-discard-control intentionally permits early payload banking even when a
  // same-turn VSTAR route is absent:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/797
  if (!sim::EngineTestAccess::play_professor_burnet(engine)) {
    throw std::runtime_error("No-discard-control banking must remain legal.");
  }
}

}  // namespace

int main() {
  try {
    test_strict_jit_holds_burnet_without_vstar_route();
    test_held_vstar_preserves_strict_jit_route();
    test_payable_treasure_preserves_strict_jit_route();
    test_no_discard_control_keeps_no_vstar_banking();
    std::cout << "Burnet no-VSTAR-route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
