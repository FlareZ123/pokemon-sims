#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
  }
  static bool redundant_held_crispin_route(const Engine& engine) {
    return engine.wonder_tag_is_redundant_with_held_crispin_legacy_star_route();
  }
  static bool needs_tapu(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-1016", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, true, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{1016};
  return sim::Engine(scenario, recipe, rng);
}

sim::Engine make_rule_box_locked_engine() {
  static const sim::Scenario scenario{"issue-1016-rule-box-lock",
                                      sim::DciProfile::StrictJit,
                                      sim::LockMode::FullRuleBoxAbility, true, 4};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{1017};
  return sim::Engine(scenario, recipe, rng);
}

sim::State complete_held_crispin_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::FieldBlower, sim::Card::RegidragoVstar,
                sim::Card::TapuLeleGX, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::QuickBall,
                sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::ProfessorBurnet,
                sim::Card::Arven, sim::Card::MysteriousTreasure,
                sim::Card::RegidragoV, sim::Card::TapuLeleGX};
  return state;
}

void test_held_crispin_legacy_star_route_suppresses_tapu_connector() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, complete_held_crispin_state());

  // Held Crispin can attach one needed Basic Energy and put the other into hand
  // for the unused manual attachment. The prior-turn Active Regidrago V can then
  // evolve and use Legacy Star, so optional Wonder Tag adds no setup axis:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
  expect(sim::EngineTestAccess::redundant_held_crispin_route(engine),
         "The complete held-Crispin Legacy Star route must make Wonder Tag redundant.");
  expect(!sim::EngineTestAccess::needs_tapu(engine),
         "The shared Tapu connector gate must reject the redundant route.");
}

void test_missing_crispin_keeps_tapu_connector_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_held_crispin_state();
  std::erase(state.hand, sim::Card::Crispin);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::redundant_held_crispin_route(engine),
         "Wonder Tag must remain live when the held Energy Supporter is absent.");
}

void test_spent_vstar_power_keeps_tapu_connector_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_held_crispin_state();
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::redundant_held_crispin_route(engine),
         "Wonder Tag must remain live when Legacy Star is unavailable.");
}

void test_fresh_regidrago_keeps_tapu_connector_live() {
  sim::Engine engine = make_engine();
  sim::State state = complete_held_crispin_state();
  state.active->entered_turn = state.turn;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(!sim::EngineTestAccess::redundant_held_crispin_route(engine),
         "Wonder Tag must remain live when Regidrago V cannot evolve this turn.");
}

void test_rule_box_ability_lock_disables_legacy_star_projection() {
  sim::Engine engine = make_rule_box_locked_engine();
  sim::EngineTestAccess::set_state(engine, complete_held_crispin_state());
  expect(!sim::EngineTestAccess::redundant_held_crispin_route(engine),
         "The complete-route projection must reject a locked Legacy Star Ability.");
}

void test_full_trace_skips_first_wonder_tag_and_quick_ball() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{136};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();
  const int tapu_in_play = static_cast<int>(std::count_if(
      state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
        return pokemon.card == sim::Card::TapuLeleGX;
      }));

  // The confirmed seed must take the direct held Crispin, manual attachment,
  // evolution, and Legacy Star route without spending either optional connector:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
  expect(outcome.first_ready_turn == 2,
         "The corrected route must preserve turn-two readiness.");
  expect(tapu_in_play == 0,
         "The redundant first Tapu Lele-GX must remain out of play.");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::TapuLeleGX) == 1,
         "The held Tapu Lele-GX must be preserved.");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::QuickBall) == 1,
         "The redundant Quick Ball must be preserved.");
}

}  // namespace

int main() {
  try {
    test_held_crispin_legacy_star_route_suppresses_tapu_connector();
    test_missing_crispin_keeps_tapu_connector_live();
    test_spent_vstar_power_keeps_tapu_connector_live();
    test_fresh_regidrago_keeps_tapu_connector_live();
    test_rule_box_ability_lock_disables_legacy_star_projection();
    test_full_trace_skips_first_wonder_tag_and_quick_ball();
    std::cout << "Issue 1016 Wonder Tag duplicate Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
