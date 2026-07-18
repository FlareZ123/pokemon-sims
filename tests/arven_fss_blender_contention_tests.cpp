#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool split_route_live(const Engine& engine) {
    return engine.arven_fss_blender_contention_route_live();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

bool has(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::DeckRecipe& test_recipe() {
  // Engine stores the recipe by reference, so this owner must outlive every test Engine:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::Arven, sim::Card::Fire};
  state.deck = {sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
                sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::Dipplin};
  return state;
}

void test_splits_vstar_and_payload_connectors() {
  const sim::Scenario scenario{"arven-fss-blender-contention", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(74501);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, base_state());

  // Arven can take Brilliant Blender plus Forest Seal Stone. Star Alchemy then
  // searches Regidrago VSTAR, Blender supplies the payload, and Latias ex supplies
  // the free-retreat route without reusing a constrained connector:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/745
  expect(sim::EngineTestAccess::split_route_live(engine),
         "The complete split route should be recognized");
  expect(sim::EngineTestAccess::play_arven(engine),
         "Arven should play on the complete split route");
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(has(state.hand, sim::Card::BrilliantBlender), "Arven should find Brilliant Blender");
  expect(has(state.hand, sim::Card::ForestSealStone), "Arven should find Forest Seal Stone");
  expect(!has(state.hand, sim::Card::MysteriousTreasure),
         "Arven should not duplicate the VSTAR connector with Mysterious Treasure");
}

void test_preserves_vstar_search_without_active_route() {
  const sim::Scenario scenario{"arven-no-active-route", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(74502);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::State state = base_state();
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Without Latias ex or another concrete same-turn promotion route, Blender cannot
  // complete readiness and must not displace the ordinary Arven policy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(!sim::EngineTestAccess::split_route_live(engine),
         "The split route must remain unavailable without an Active-position route");
}

void test_preserves_vstar_search_for_new_regidrago() {
  const sim::Scenario scenario{"arven-new-regi", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(74503);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::State state = base_state();
  state.bench.front().entered_turn = 3;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A Basic Pokemon cannot evolve during the turn it entered play, so the split
  // route is unavailable even though Latias ex can later provide free retreat:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::split_route_live(engine),
         "The split route must reject a Regidrago V that entered play this turn");
}

void test_preserves_energy_axis_priority() {
  const sim::Scenario scenario{"arven-energy-incomplete", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(74504);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::State state = base_state();
  state.bench.front().grass = 1;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The Blender/FSS split is valid only after the selected Regidrago line already
  // pays Apex Dragon's GGF cost. Energy completion remains the higher priority axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(!sim::EngineTestAccess::split_route_live(engine),
         "The split route must reject an Energy-incomplete Regidrago V");
}

}  // namespace

int main() {
  test_splits_vstar_and_payload_connectors();
  test_preserves_vstar_search_without_active_route();
  test_preserves_vstar_search_for_new_regidrago();
  test_preserves_energy_axis_priority();
  std::cout << "arven_fss_blender_contention_tests: all checks passed\n";
}
