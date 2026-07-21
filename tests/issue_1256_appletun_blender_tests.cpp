#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play(Engine& engine) { return engine.play_brilliant_blender(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1256/appletun-blender", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1256};
  sim::Engine engine{scenario, sim::pineco_recipe(), rng};
};

sim::State ready_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Appletun};
  return state;
}

void test_appletun_only_payload_resolves() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, ready_state());

  // Brilliant Blender may discard up to five cards from the deck. Appletun is a
  // Dragon Pokémon whose attack is a legal Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1256
  if (!sim::EngineTestAccess::play(fixture.engine)) {
    throw std::runtime_error("Brilliant Blender rejected Appletun-only payload state.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Brilliant Blender consumed itself without Appletun.");
  }
}

void test_item_lock_preserves_cards() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1257};
  sim::Engine engine{fixture.scenario, sim::pineco_recipe(), rng};
  const sim::State before = ready_state();
  sim::EngineTestAccess::set_state(engine, before);
  if (sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Brilliant Blender resolved through Item lock.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != before.hand || after.deck != before.deck ||
      !after.discard.empty()) {
    throw std::runtime_error("Rejected Item-lock state mutated cards.");
  }
}

}  // namespace

int main() {
  test_appletun_only_payload_resolves();
  test_item_lock_preserves_cards();
  std::cout << "Issue 1256 Appletun Brilliant Blender tests passed.\n";
  return 0;
}
