#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1257/appletun-burnet",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1257};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State ready_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet};
  return state;
}

void test_appletun_only_payload_resolves() {
  Fixture fixture;
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::Appletun};

  // Professor Burnet searches the deck and discards up to two cards. Appletun
  // sv8-140 is a Dragon Pokémon whose attack is a legal Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1257
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("Professor Burnet rejected an Appletun-only payload state.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used ||
      !contains(after.discard, sim::Card::ProfessorBurnet) ||
      !contains(after.discard, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error("Professor Burnet spent the Supporter turn without discarding Appletun.");
  }
}

void test_as_payload_priority_precedes_dipplin() {
  Fixture fixture;
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::MegaDragonite, sim::Card::Appletun,
                sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_professor_burnet(fixture.engine)) {
    throw std::runtime_error("Professor Burnet rejected the payload-priority state.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discard, sim::Card::Appletun) ||
      !contains(after.deck, sim::Card::Dipplin) ||
      contains(after.discard, sim::Card::Dipplin)) {
    throw std::runtime_error("Dipplin displaced Appletun from Burnet's two-card limit.");
  }
}

void test_supporter_lock_preserves_burnet_and_appletun() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullSupporter;
  std::mt19937_64 rng{1258};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  sim::State state = ready_vstar_state();
  state.deck = {sim::Card::Appletun};
  const auto hand_before = state.hand;
  const auto deck_before = state.deck;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (sim::EngineTestAccess::play_professor_burnet(engine)) {
    throw std::runtime_error("Professor Burnet resolved through Supporter lock.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand != hand_before || after.deck != deck_before ||
      !after.discard.empty() || after.supporter_used) {
    throw std::runtime_error("Rejected Supporter-lock state mutated Burnet or Appletun.");
  }
}

}  // namespace

int main() {
  test_appletun_only_payload_resolves();
  test_as_payload_priority_precedes_dipplin();
  test_supporter_lock_preserves_burnet_and_appletun();
  std::cout << "Issue 1257 Appletun Professor Burnet tests passed.\n";
  return 0;
}
