#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <optional>
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

  static std::optional<Card> treasure_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true,
                                 Card::MysteriousTreasure, false);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"issue-1205/preserve-turo",
                         sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, true, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1205};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State exact_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::DialgaGX,
                sim::Card::Dragapult, sim::Card::ProfessorTuro,
                sim::Card::ForestSealStone, sim::Card::Crispin,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoV, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect_cost(sim::Engine& engine, sim::State state,
                 const sim::Card expected, const char* message) {
  sim::EngineTestAccess::set_state(engine, std::move(state));
  if (sim::EngineTestAccess::treasure_cost(engine) != expected) {
    throw std::runtime_error(message);
  }
}

void test_redundant_payload_precedes_only_turo_connector() {
  Fixture fixture;
  // Mysterious Treasure may discard any other held card. A discarded Dragon
  // remains an Apex Dragon attack source, while Professor Turo is the only public
  // Active-position connector in this exact state:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1205
  expect_cost(fixture.engine, exact_state(), sim::Card::Dragapult,
              "Redundant Dragon did not precede the sole Turo connector.");
}

void test_single_payload_stays_protected() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.erase(state.hand.begin() + 2);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "A lone payload was spent to preserve Turo.");
}

void test_active_regidrago_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected without a stranded Active.");
}

void test_latias_connector_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.push_back(sim::Card::LatiasEx);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a live Latias connector.");
}

void test_tate_connector_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.push_back(sim::Card::TateLiza);
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a held Tate & Liza connector.");
}

void test_retreat_energy_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.active->grass = 1;
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected despite a paid retreat route.");
}

void test_full_bench_keeps_turo_expendable() {
  Fixture fixture;
  sim::State state = exact_state();
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::MawileGX, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::Pineco, 0, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::ForretressEx, 0, 0, 0, sim::Tool::None}};
  expect_cost(fixture.engine, std::move(state), sim::Card::ProfessorTuro,
              "Turo was protected when no Regidrago Bench slot existed.");
}

void test_item_lock_keeps_turo_expendable() {
  Fixture fixture;
  fixture.scenario.locks = sim::LockMode::FullItem;
  std::mt19937_64 rng{1206};
  sim::Engine engine{fixture.scenario, fixture.recipe, rng};
  expect_cost(engine, exact_state(), sim::Card::ProfessorTuro,
              "Treasure-specific protection activated through Item lock.");
}

}  // namespace

int main() {
  test_redundant_payload_precedes_only_turo_connector();
  test_single_payload_stays_protected();
  test_active_regidrago_keeps_turo_expendable();
  test_latias_connector_keeps_turo_expendable();
  test_tate_connector_keeps_turo_expendable();
  test_retreat_energy_keeps_turo_expendable();
  test_full_bench_keeps_turo_expendable();
  test_item_lock_keeps_turo_expendable();
  std::cout << "Issue 1205 Turo-preservation tests passed.\n";
  return 0;
}
