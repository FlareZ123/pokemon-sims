#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool play_turo_promotion(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
  static bool held_turo_promotion(const Engine& engine) {
    return engine.held_turo_direct_active_promotion_route();
  }
  static std::optional<Card> turo_oricorio_basic(const Engine& engine) {
    return engine.turo_oricorio_finishing_basic_energy();
  }
  static bool turo_oricorio_live(const Engine& engine) {
    return engine.turo_oricorio_energy_route_live();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool pays_apex(const Engine& engine, const Pokemon& pokemon) {
    return engine.pays_apex_energy_cost(pokemon);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Pokemon pokemon(const sim::Card card, const int grass = 0,
                     const int fire = 0, const int dde = 0,
                     const int entered_turn = 1) {
  sim::Pokemon result{card, entered_turn, grass, fire, sim::Tool::None};
  result.double_dragon = dde;
  return result;
}

sim::State payload_ready_state() {
  sim::State state;
  state.turn = 3;
  state.discard = {sim::Card::Dragapult};
  state.discarded_this_turn = {sim::Card::Dragapult};
  return state;
}

struct Fixture {
  sim::Scenario scenario{"issue-2418", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::double_dragon_modeling_recipe()};
  std::mt19937_64 rng{2418};
  sim::Engine engine{scenario, recipe, rng};
};

void test_benched_dde_basic_vstar_promotion(const int grass, const int fire) {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::Oricorio);
  state.bench = {pokemon(sim::Card::RegidragoVstar, grass, fire, 1)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // DDE supplies two Energy of every type while attached to a Dragon. One Basic
  // Grass or Fire therefore makes the Benched VSTAR pay Apex Dragon's GGF cost.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Official Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo rejected a DDE-plus-Basic complete Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar,
         "Turo did not promote the DDE-complete Regidrago VSTAR.");
  expect(sim::EngineTestAccess::pays_apex(fixture.engine, *after.active),
         "Promoted DDE VSTAR does not semantically pay Apex Dragon.");
}

void test_direct_dde_evolution_suppresses_turo() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoV, 1, 0, 1, 1);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 0, 1, 1)};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A prior-turn Regidrago V with DDE + Grass already pays Apex. The held VSTAR
  // can evolve it in place, so spending Turo and returning attached cards is weaker.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(!sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo preempted the direct DDE-complete evolution route.");
  expect(!sim::EngineTestAccess::held_turo_promotion(fixture.engine),
         "Duplicate Turo readiness helper ignored the direct DDE evolution.");
}

void test_duplicate_turo_helper_accepts_dde_benched_vstar() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 0, 1, 1)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The duplicate Wonder Tag/Turo guard must use the same semantic ready attacker.
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::held_turo_promotion(fixture.engine),
         "Duplicate Turo helper rejected DDE + Fire Benched VSTAR.");
}

void test_turo_discards_attached_dde() {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoV, 1, 0, 1, 1);
  state.active->tool = sim::Tool::ForestSealStone;
  state.bench = {pokemon(sim::Card::RegidragoVstar, 2, 1, 0)};
  state.hand = {sim::Card::ProfessorTuro};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Turo returns the Pokémon and discards every attached card. The physical DDE,
  // Basic Grass, and Tool must all enter public discard rather than disappear.
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Official Supporter/discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::play_turo_promotion(fixture.engine),
         "Turo discard-state fixture did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::DoubleDragonEnergy) == 1,
         "Attached DDE disappeared instead of entering discard.");
  expect(std::count(after.discarded_this_turn.begin(), after.discarded_this_turn.end(),
                    sim::Card::DoubleDragonEnergy) == 1,
         "Attached DDE was not recorded in discarded_this_turn.");
  expect(std::count(after.discard.begin(), after.discard.end(), sim::Card::Grass) == 1,
         "Attached Basic Grass disappeared during Turo return.");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::ForestSealStone) == 1,
         "Attached Forest Seal Stone disappeared during Turo return.");
}

void test_turo_oricorio_projection_accepts_either_basic(const sim::Card basic) {
  Fixture fixture;
  sim::State state = payload_ready_state();
  state.active = pokemon(sim::Card::RegidragoVstar, 0, 0, 1);
  state.bench = {pokemon(sim::Card::Oricorio)};
  state.hand = {sim::Card::ProfessorTuro};
  state.deck = {basic};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Vital Dance searches Basic Energy. With one DDE already attached, either Basic
  // type is a legal one-attachment Apex completion and must survive route projection.
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(sim::EngineTestAccess::turo_oricorio_basic(fixture.engine) == basic,
         "Turo-Oricorio did not project the DDE one-Basic completion.");
  expect(sim::EngineTestAccess::turo_oricorio_live(fixture.engine),
         "Turo-Oricorio route rejected the DDE one-Basic completion.");
}

void test_issue_1595_preserves_quick_ball_for_dde_turo() {
  sim::Scenario scenario{"issue-2418/1595", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, true, 5};
  std::mt19937_64 rng(24181595);
  sim::Engine engine(scenario, sim::double_dragon_modeling_recipe(), rng);
  sim::State state;
  state.turn = 4;
  state.active = pokemon(sim::Card::TapuLeleGX);
  state.bench = {pokemon(sim::Card::RegidragoVstar, 1, 0, 1)};
  state.hand = {sim::Card::ProfessorTuro, sim::Card::BrilliantBlender,
                sim::Card::QuickBall};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Held Turo already owns the Active-position axis and Blender owns payload.
  // Spending Quick Ball has no marginal setup value when the Benched DDE VSTAR
  // already pays Apex Dragon's cost.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2418
  expect(!sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball was spent instead of being held for direct Turo promotion.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::find(after.hand.begin(), after.hand.end(), sim::Card::QuickBall) !=
             after.hand.end(),
         "Quick Ball left hand in the DDE-complete Turo state.");
}

}  // namespace

int main() {
  try {
    test_benched_dde_basic_vstar_promotion(1, 0);
    test_benched_dde_basic_vstar_promotion(0, 1);
    test_direct_dde_evolution_suppresses_turo();
    test_duplicate_turo_helper_accepts_dde_benched_vstar();
    test_turo_discards_attached_dde();
    test_turo_oricorio_projection_accepts_either_basic(sim::Card::Grass);
    test_turo_oricorio_projection_accepts_either_basic(sim::Card::Fire);
    test_issue_1595_preserves_quick_ball_for_dde_turo();
    std::cout << "Issue 2418 DDE Turo tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
