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
  static bool play(Engine& engine) { return engine.play_professor_burnet(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1257/burnet-appletun", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1257};
  sim::Engine engine{scenario, sim::pineco_recipe(), rng};
};

sim::State ready_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Appletun};
  return state;
}

void test_appletun_only_payload_resolves() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, ready_state());

  // Professor Burnet may search for and discard up to two cards. Appletun is a
  // Dragon Pokémon whose attack is a legal Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1257
  if (!sim::EngineTestAccess::play(fixture.engine)) {
    throw std::runtime_error("Professor Burnet rejected Appletun-only payload state.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used ||
      !contains(after.discard, sim::Card::ProfessorBurnet) ||
      !contains(after.discard, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Professor Burnet spent the Supporter without Appletun.");
  }
}

void test_supporter_lock_preserves_cards() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullSupporter;
  std::mt19937_64 rng{1258};
  sim::Engine engine{fixture.scenario, sim::pineco_recipe(), rng};
  const sim::State before = ready_state();
  sim::EngineTestAccess::set_state(engine, before);
  if (sim::EngineTestAccess::play(engine)) {
    throw std::runtime_error("Professor Burnet resolved through Supporter lock.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != before.hand || after.deck != before.deck ||
      !after.discard.empty() || after.supporter_used) {
    throw std::runtime_error("Rejected Supporter-lock state mutated cards.");
  }
}

}  // namespace

int main() {
  test_appletun_only_payload_resolves();
  test_supporter_lock_preserves_cards();
  std::cout << "Issue 1257 Professor Burnet Appletun tests passed.\n";
  return 0;
}
