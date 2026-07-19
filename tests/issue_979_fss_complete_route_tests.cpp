#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool should_hold_complete_route(const Engine& engine) {
    return engine.fss_should_hold_for_complete_quick_ball_route();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None,
                   const std::uint64_t seed = 979)
      : scenario{"issue-979-complete-route", sim::DciProfile::StrictJit,
                 lock, true, 5},
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State complete_route_state() {
  sim::State state;
  state.turn = 4;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                   sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::QuickBall, sim::Card::Dragapult,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::LatiasEx, sim::Card::ProfessorBurnet,
                sim::Card::TapuLeleGX, sim::Card::Grass};
  return state;
}

void test_complete_arven_route_holds_star_alchemy() {
  // Arven has used the turn's Supporter play, so a searched Professor Burnet cannot
  // resolve this turn: https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Quick Ball can discard the held Dragon and search Latias ex, which supplies the
  // free-retreat route after the prior-turn GGF Regidrago V evolves:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // The held ACE SPEC remains a second complete payload route, so the once-per-game
  // Star Alchemy should be preserved: https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/979
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_route_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the exact Arven route should satisfy the narrow Star Alchemy hold predicate");
  expect(!sim::EngineTestAccess::use_fss(fixture.engine),
         "Star Alchemy should be held when every current-turn axis is already solved");
  expect(!sim::EngineTestAccess::state(fixture.engine).vstar_power_used,
         "the once-per-game VSTAR Power must remain unused");
}

void test_absent_blender_does_not_enter_new_hold_predicate() {
  // The regression is intentionally narrow to the state with the held singleton
  // Blender documented by issue #979: https://api.pokemontcg.io/v2/cards/sv8-164
  Fixture fixture{sim::LockMode::None, 980};
  sim::State state = complete_route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::BrilliantBlender));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(!sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the new hold predicate must remain inactive without held Blender");
}

void test_missing_quick_ball_route_does_not_enter_new_hold_predicate() {
  // Quick Ball is the legal same-turn discard and Latias connector in this route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  Fixture fixture{sim::LockMode::None, 981};
  sim::State state = complete_route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::QuickBall));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(!sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the new hold predicate must remain inactive without Quick Ball");
}

void test_missing_payload_cost_does_not_enter_new_hold_predicate() {
  // Quick Ball requires a discard cost, and strict JIT requires the Dragon payload
  // to enter discard during the ready turn: https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  Fixture fixture{sim::LockMode::None, 982};
  sim::State state = complete_route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Dragapult));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(!sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the new hold predicate must remain inactive without a held Dragon payload");
}

void test_missing_latias_target_does_not_enter_new_hold_predicate() {
  // Quick Ball must retain a legal Basic target, and this route specifically needs
  // Latias ex for Active-position completion:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  Fixture fixture{sim::LockMode::None, 983};
  sim::State state = complete_route_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::LatiasEx));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(!sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the new hold predicate must remain inactive without the Latias target");
}

void test_item_lock_does_not_enter_new_hold_predicate() {
  // Item lock makes the Quick Ball continuation unavailable, so the #979 hold
  // predicate must stay out of the legacy Forest Seal Stone selector:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/979
  Fixture fixture{sim::LockMode::FullItem, 984};
  sim::EngineTestAccess::set_state(fixture.engine, complete_route_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  expect(!sim::EngineTestAccess::should_hold_complete_route(fixture.engine),
         "the new hold predicate must remain inactive under Item lock");
}

}  // namespace

int main() {
  try {
    test_complete_arven_route_holds_star_alchemy();
    test_absent_blender_does_not_enter_new_hold_predicate();
    test_missing_quick_ball_route_does_not_enter_new_hold_predicate();
    test_missing_payload_cost_does_not_enter_new_hold_predicate();
    test_missing_latias_target_does_not_enter_new_hold_predicate();
    test_item_lock_does_not_enter_new_hold_predicate();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
