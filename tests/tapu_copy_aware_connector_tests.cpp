#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
  static bool bench_tapu_if_useful(Engine& engine) {
    return engine.bench_tapu_if_useful();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool might_be_unseen(const Engine& engine, const Card card) {
    return engine.might_be_unseen(card);
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"tapu-copy-aware-connectors",
                         sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{746};
  sim::Engine engine{scenario, recipe, rng};
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Pokemon>& pokemon, const sim::Card card) {
  return static_cast<int>(std::count_if(
      pokemon.begin(), pokemon.end(),
      [card](const sim::Pokemon& value) { return value.card == card; }));
}

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

sim::State prized_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None}};
  state.prizes = {sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Gladion, sim::Card::Arven};
  return state;
}

void test_second_held_copy_resolves_wonder_tag() {
  Fixture fixture;
  sim::State state = prized_vstar_state();
  state.hand = {sim::Card::TapuLeleGX};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);

  // Wonder Tag is an entry Ability of the individual Tapu Lele-GX played from hand.
  // The setup copy does not exhaust a second held copy's Ability:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/746
  expect(sim::EngineTestAccess::needs_tapu_connector(fixture.engine),
         "The prized VSTAR must expose a live Gladion Wonder Tag route.");
  expect(sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine),
         "The second held Tapu must be Benched and resolve Wonder Tag.");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->card == sim::Card::TapuLeleGX,
         "The setup Tapu must remain Active.");
  expect(count(after.bench, sim::Card::TapuLeleGX) == 1,
         "The second physical Tapu must be on the Bench.");
  expect(contains(after.hand, sim::Card::Gladion),
         "Wonder Tag must find the live Gladion connector.");
}

void test_quick_ball_can_find_second_copy() {
  Fixture fixture;
  sim::State state = prized_vstar_state();
  state.hand = {sim::Card::QuickBall, sim::Card::Fire, sim::Card::Fire};
  state.deck.insert(state.deck.begin(), sim::Card::TapuLeleGX);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);

  // Quick Ball may search the second physical Basic even when another Tapu is in
  // play, after which that fetched copy may be Benched for its own Wonder Tag:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/issues/746
  expect(sim::EngineTestAccess::play_quick_ball(fixture.engine),
         "Quick Ball must find the second Tapu copy.");
  expect(contains(sim::EngineTestAccess::state(fixture.engine).hand,
                  sim::Card::TapuLeleGX),
         "The searched second Tapu must enter hand.");
  expect(sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine),
         "The searched second Tapu must resolve Wonder Tag.");
  expect(contains(sim::EngineTestAccess::state(fixture.engine).hand,
                  sim::Card::Gladion),
         "The searched-copy Wonder Tag must find Gladion.");
}

void test_dead_target_holds_second_copy() {
  Fixture fixture;
  sim::State state = prized_vstar_state();
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true);

  // Copy-aware eligibility does not create a connector when the inspected deck has
  // no live Supporter target: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/746
  expect(!sim::EngineTestAccess::needs_tapu_connector(fixture.engine),
         "A dead Wonder Tag target must keep the connector closed.");
  expect(!sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine),
         "The second Tapu must remain held without a live target.");
}

void test_two_public_copies_close_fixed_list_availability() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None}};
  state.deck = {sim::Card::Gladion, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false);

  // The fixed recipe contains two Tapu Lele-GX. Once both physical copies are public,
  // K0 must not invent a third searchable copy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_000.inc#L75-L88
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/746
  expect(!sim::EngineTestAccess::might_be_unseen(
             fixture.engine, sim::Card::TapuLeleGX),
         "Both public copies must close Tapu fixed-list uncertainty.");
}

}  // namespace

int main() {
  test_second_held_copy_resolves_wonder_tag();
  test_quick_ball_can_find_second_copy();
  test_dead_target_holds_second_copy();
  test_two_public_copies_close_fixed_list_availability();
  std::cout << "All copy-aware Tapu connector tests passed.\n";
  return 0;
}
