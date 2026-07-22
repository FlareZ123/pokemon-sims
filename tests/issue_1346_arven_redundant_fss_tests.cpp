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
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::RegidragoVstar, sim::Card::Grass};
  state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar,
                sim::Card::BrilliantBlender, sim::Card::Dragapult,
                sim::Card::Dipplin};
  return state;
}

void test_held_vstar_preserves_forest_seal_stone() {
  const sim::Scenario scenario{"issue-1346-held-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(134601);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, base_state());

  // Arven may search Brilliant Blender while leaving its separate Tool search
  // unresolved. The held Regidrago VSTAR already completes the card axis, so
  // Forest Seal Stone and the Active Tool slot have no current setup role:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1346
  expect(sim::EngineTestAccess::play_arven(engine), "Arven should find the live Blender route");
  const sim::State& state = sim::EngineTestAccess::state(engine);
  expect(has(state.hand, sim::Card::BrilliantBlender), "Arven should find Brilliant Blender");
  expect(!has(state.hand, sim::Card::ForestSealStone),
         "Arven should preserve redundant Forest Seal Stone in deck");
  expect(has(state.deck, sim::Card::ForestSealStone),
         "Forest Seal Stone should remain available in deck");
}

void test_missing_vstar_keeps_original_split_route() {
  const sim::Scenario scenario{"issue-1346-missing-vstar", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(134602);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::State state = base_state();
  state.hand = {sim::Card::Arven, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Without a held Regidrago VSTAR, the established #745 route still needs both
  // Brilliant Blender and Forest Seal Stone so Star Alchemy can search the VSTAR:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/745
  // https://github.com/FlareZ123/pokemon-sims/issues/1346
  expect(sim::EngineTestAccess::play_arven(engine), "Arven should keep the split connector route");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(has(result.hand, sim::Card::BrilliantBlender), "Arven should find Brilliant Blender");
  expect(has(result.hand, sim::Card::ForestSealStone),
         "Arven should find Forest Seal Stone when Star Alchemy supplies VSTAR");
}

}  // namespace

int main() {
  test_held_vstar_preserves_forest_seal_stone();
  test_missing_vstar_keeps_original_split_route();
  std::cout << "issue_1346_arven_redundant_fss_tests: all checks passed\n";
}
