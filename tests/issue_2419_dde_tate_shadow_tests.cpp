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
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool tate_held_completion(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe dde_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto grass = std::find_if(recipe.begin(), recipe.end(), [](const auto& entry) {
    return entry.first == sim::Card::Grass;
  });
  expect(grass != recipe.end() && grass->second >= 2,
         "Baseline Grass count cannot supply the DDE test recipe.");
  const auto index = std::distance(recipe.begin(), grass);
  --grass->second;
  recipe.insert(recipe.begin() + index, {sim::Card::DoubleDragonEnergy, 1});
  return recipe;
}

sim::Pokemon ready_vstar(const int dde) {
  sim::Pokemon pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};
  pokemon.double_dragon = dde;
  return pokemon;
}

sim::State blender_payload_state(const int dde) {
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar(dde);
  state.hand = {sim::Card::TateLiza, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Dragapult, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_dde_complete_shadow_preserves_tate() {
  const sim::Scenario scenario{
      "issue-2419/dde-shadow", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  std::mt19937_64 rng(2419);
  sim::Engine engine(scenario, dde_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, blender_payload_state(1));

  // Brilliant Blender is a held non-Supporter payload completion. G + F + DDE
  // already pays Apex Dragon's GGF cost, so Tate & Liza must not shuffle away
  // the deterministic current-turn finish.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Official Supporter, Item, Energy, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // DDE semantic-readiness specification: https://github.com/FlareZ123/pokemon-sims/issues/2238
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2419
  expect(sim::EngineTestAccess::tate_held_completion(engine),
         "DDE-complete held Blender route did not preserve Tate & Liza.");
}

void test_unpowered_control_still_allows_tate() {
  const sim::Scenario scenario{
      "issue-2419/control", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  std::mt19937_64 rng(241901);
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, blender_payload_state(0));

  // One Grass plus one Fire without DDE does not pay GGF. Blender alone solves
  // payload only, so this state must not be misclassified as a complete held route.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/2419
  expect(!sim::EngineTestAccess::tate_held_completion(engine),
         "Unpowered GF control was incorrectly classified as a complete held route.");
}

}  // namespace

int main() {
  try {
    test_dde_complete_shadow_preserves_tate();
    test_unpowered_control_still_allows_tate();
    std::cout << "Issue 2419 DDE Tate shadow tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
