#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

struct Fixture {
  explicit Fixture(const sim::Scenario& selected_scenario)
      : scenario(selected_scenario), recipe(sim::baseline_recipe()), rng(725),
        engine(scenario, recipe, rng) {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

sim::State powered_benched_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::BrilliantBlender};
  state.deck = {sim::Card::MegaDragonite};
  return state;
}

void test_holds_blender_without_same_turn_promotion_route() {
  Fixture fixture({"blender-active-axis-hold", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  const sim::State before = state;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Brilliant Blender's deck discard expires at turn end under strict JIT. A
  // powered Benched Regidrago VSTAR still cannot attack while Oricorio remains
  // Active, so preserve the one-copy ACE SPEC until a modeled promotion route exists:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(!sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender should be held when the Active axis cannot finish this turn");

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.hand == before.hand, "Blender should remain in hand");
  expect(after.deck == before.deck, "the payload should remain in deck");
  expect(after.discard.empty(), "no one-shot resource should be spent");
}

void test_tate_route_keeps_blender_live() {
  Fixture fixture({"blender-tate-promotion", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  state.hand.push_back(sim::Card::TateLiza);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Tate & Liza can switch the powered Benched VSTAR Active during this turn:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender should remain live with Tate & Liza held");
}

void test_latias_route_keeps_blender_live() {
  Fixture fixture({"blender-latias-promotion", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  state.bench.push_back(
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None});
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Skyliner gives the Basic Active no Retreat Cost, so the VSTAR has a concrete
  // same-turn promotion route:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender should remain live with an available Latias route");
}

void test_forest_seal_stone_latias_route_keeps_blender_live() {
  Fixture fixture({"blender-fss-latias-promotion", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  state.bench.front().tool = sim::Tool::ForestSealStone;
  state.deck.push_back(sim::Card::LatiasEx);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Star Alchemy may search Latias ex, whose Skyliner Ability gives the Basic
  // Active no Retreat Cost. This is a concrete modeled same-turn route:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender should remain live with an available FSS-to-Latias route");
}

void test_unknown_guzma_route_holds_blender() {
  Fixture fixture({"blender-unknown-guzma", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4,
                   sim::OpponentBenchState::Unknown});
  sim::State state = powered_benched_vstar_state();
  state.hand.push_back(sim::Card::Guzma);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Guzma's user-side switch is conditional on switching an opposing Benched
  // Pokémon. Unknown opponent state cannot prove that prerequisite:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  expect(!sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "unknown opponent state must not make Guzma a live promotion route");
}

void test_available_guzma_route_keeps_blender_live() {
  Fixture fixture({"blender-available-guzma", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4,
                   sim::OpponentBenchState::Available});
  sim::State state = powered_benched_vstar_state();
  state.hand.push_back(sim::Card::Guzma);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // An explicit Available opponent Bench satisfies Guzma's conditional first
  // switch, so the merged #741 resolver can promote the powered VSTAR after Blender:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "explicitly available Guzma route should keep Blender live");
}

void test_already_active_vstar_keeps_blender_live() {
  Fixture fixture({"blender-active-vstar-control", sim::DciProfile::StrictJit,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.bench.clear();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // An already-Active Regidrago VSTAR at GGF can use the same-turn payload:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "Blender should remain live when the VSTAR is already Active");
}

void test_no_discard_control_behavior_is_unchanged() {
  Fixture fixture({"blender-active-axis-no-control",
                   sim::DciProfile::NoDiscardControl,
                   sim::LockMode::None, false, 4});
  sim::State state = powered_benched_vstar_state();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The new hold is specific to strict current-turn timing:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/725
  expect(sim::EngineTestAccess::play_brilliant_blender(fixture.engine),
         "no-discard-control should preserve its existing early banking behavior");
}

}  // namespace

int main() {
  try {
    test_holds_blender_without_same_turn_promotion_route();
    test_tate_route_keeps_blender_live();
    test_latias_route_keeps_blender_live();
    test_forest_seal_stone_latias_route_keeps_blender_live();
    test_unknown_guzma_route_holds_blender();
    test_available_guzma_route_keeps_blender_live();
    test_already_active_vstar_keeps_blender_live();
    test_no_discard_control_behavior_is_unchanged();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
