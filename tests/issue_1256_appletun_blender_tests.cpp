#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1256/appletun-blender",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1256};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State ready_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender};
  return state;
}

void test_appletun_only_payload_resolves() {
  Fixture fixture;
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::Appletun};

  // Brilliant Blender may search the deck and discard up to five cards, while
  // Appletun sv8-140 is a Dragon Pokémon whose attack is a legal Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1256
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error("Brilliant Blender rejected an Appletun-only payload state.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Brilliant Blender consumed itself without discarding Appletun.");
  }
}

void test_as_payload_priority_precedes_dipplin() {
  Fixture fixture;
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::GoodraVstar, sim::Card::DialgaGX,
                sim::Card::Appletun, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_brilliant_blender(fixture.engine)) {
    throw std::runtime_error("Brilliant Blender rejected the complete payload-priority state.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  for (const sim::Card payload : {sim::Card::MegaDragonite,
                                  sim::Card::Dragapult,
                                  sim::Card::GoodraVstar,
                                  sim::Card::DialgaGX,
                                  sim::Card::Appletun}) {
    if (!contains(after.discard, payload)) {
      throw std::runtime_error("An A/S payload lost priority inside Brilliant Blender.");
    }
  }
  if (!contains(after.deck, sim::Card::Dipplin) ||
      contains(after.discard, sim::Card::Dipplin)) {
    throw std::runtime_error("Dipplin displaced an A/S payload from Blender's five-card limit.");
  }
}

void test_item_lock_preserves_blender_and_appletun() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1257};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::Appletun};
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (sim::EngineTestAccess::play_brilliant_blender(engine)) {
    throw std::runtime_error("Brilliant Blender resolved through full Item lock.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      !after.discard.empty()) {
    throw std::runtime_error("Rejected Item-lock state mutated Blender or Appletun.");
  }
}

}  // namespace

int main() {
  test_appletun_only_payload_resolves();
  test_as_payload_priority_precedes_dipplin();
  test_item_lock_preserves_blender_and_appletun();
  std::cout << "Issue 1256 Appletun Brilliant Blender tests passed.\n";
  return 0;
}
