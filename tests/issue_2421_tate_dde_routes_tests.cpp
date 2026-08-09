#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }
  static int apex_progress(const Engine& engine, const Pokemon& pokemon) {
    return engine.apex_energy_progress(pokemon);
  }
  static bool pays_apex(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static bool attach_projected(const Engine& engine, Pokemon& pokemon, const Card energy) {
    return engine.attach_energy_card(pokemon, energy);
  }
  static bool tate_draw_route(const Engine& engine) {
    return engine.tate_draw_after_active_vstar_evolution_route();
  }
  static bool tate_held_switch_route(const Engine& engine) {
    return engine.tate_switch_after_held_vstar_evolution_route();
  }
  static bool issue_2315_switch(const Engine& engine) {
    return engine.issue_2315_tate_switch_finishes_ready_turn();
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool manual_used(const Engine& engine) { return engine.state_.manual_energy_used; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon regi(const sim::Card card, const int entered_turn, const int grass,
                  const int fire, const int dde) {
  sim::Pokemon result{card, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

struct Fixture {
  sim::Scenario scenario{"issue-2421", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2421};
  sim::Engine engine{scenario, recipe, rng};
};

void test_dde_only_is_one_physical_basic_short() {
  Fixture fixture;
  const auto target = regi(sim::Card::RegidragoVstar, 1, 0, 0, 1);
  // One DDE provides two units of every type; one Basic Grass OR Fire completes GGF.
  // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
  expect(sim::EngineTestAccess::apex_progress(fixture.engine, target) == 2,
         "DDE-only was not classified one attachment short");
  for (const sim::Card basic : {sim::Card::Grass, sim::Card::Fire}) {
    auto projected = target;
    expect(sim::EngineTestAccess::attach_projected(fixture.engine, projected, basic),
           "Basic projection could not attach");
    expect(sim::EngineTestAccess::pays_apex(fixture.engine, projected),
           "DDE plus projected Basic did not pay Apex");
  }
}

void test_tate_draw_accepts_dde_ready_active_v() {
  sim::Scenario scenario{"issue-2421-draw", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 4};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{24211};
  sim::Engine engine{scenario, recipe, rng};
  sim::State state;
  state.turn = 2;
  state.active = regi(sim::Card::RegidragoV, 1, 1, 0, 1);
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::tate_draw_route(engine),
         "Tate draw route missed prior-turn DDE plus Basic Active Regidrago V");
}

void test_tate_held_switch_accepts_dde_ready_benched_v() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::TapuLeleGX, 1, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoV, 1, 0, 1, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::RegidragoVstar,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::tate_held_switch_route(fixture.engine),
         "Held Tate switch route missed DDE plus Basic Benched Regidrago V");
}

void test_tate_coordinator_preserves_attachment_when_target_ready() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::RegidragoV, 3, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoVstar, 1, 1, 0, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::BrilliantBlender,
                sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::attach_manual(fixture.engine),
         "Tate coordinator spent an attachment on a DDE-ready VSTAR");
  expect(!sim::EngineTestAccess::manual_used(fixture.engine),
         "Tate coordinator marked the preserved attachment used");
}

void test_issue_2315_accepts_dde_only_plus_held_basic() {
  Fixture fixture;
  sim::State state;
  state.turn = 3;
  state.active = regi(sim::Card::DialgaGX, 1, 0, 0, 0);
  state.bench = {regi(sim::Card::RegidragoVstar, 1, 0, 0, 1)};
  state.hand = {sim::Card::TateLiza, sim::Card::Grass};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::issue_2315_switch(fixture.engine),
         "Post-payload Tate switch missed DDE-only plus held Basic completion");
}
}  // namespace

int main() {
  try {
    test_dde_only_is_one_physical_basic_short();
    test_tate_draw_accepts_dde_ready_active_v();
    test_tate_held_switch_accepts_dde_ready_benched_v();
    test_tate_coordinator_preserves_attachment_when_target_ready();
    test_issue_2315_accepts_dde_only_plus_held_basic();
    std::cout << "Issue 2421 Tate DDE route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
