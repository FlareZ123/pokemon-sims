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
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1878_vessel_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1878_vessel_quick_ball_tapu_crispin_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::QuickBall,
                sim::Card::Dragapult, sim::Card::StevensResolve,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

void test_issue_1878_uses_shared_current_turn_jit_semantics() {
  std::mt19937_64 rng(2756);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict{"issue-2756-strict", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 5};
  const sim::Scenario flex{"issue-2756-flex", sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, false, 5};
  const sim::Scenario control{"issue-2756-control", sim::DciProfile::NoDiscardControl,
                              sim::LockMode::None, false, 5};
  sim::Engine strict_engine(strict, recipe, rng);
  sim::Engine flex_engine(flex, recipe, rng);
  sim::Engine control_engine(control, recipe, rng);
  sim::EngineTestAccess::set_state(strict_engine, route_state());
  sim::EngineTestAccess::set_state(flex_engine, route_state());
  sim::EngineTestAccess::set_state(control_engine, route_state());

  // Both JIT profiles require the Dragon payload to enter discard on the same
  // ready turn, so this exact K1 Vessel -> Quick Ball -> Tapu -> Crispin route
  // has identical eligibility in StrictJit and MatchupFlexJit. NoDiscardControl
  // permits earlier payload banking and remains outside this JIT-specific route.
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2756
  expect(sim::EngineTestAccess::route_available(strict_engine),
         "StrictJit lost the issue-1878 route.");
  expect(sim::EngineTestAccess::route_available(flex_engine),
         "MatchupFlexJit still rejects the issue-1878 route.");
  expect(!sim::EngineTestAccess::route_available(control_engine),
         "NoDiscardControl incorrectly entered the JIT-specific route.");

  expect(sim::EngineTestAccess::complete_route(flex_engine),
         "MatchupFlexJit could detect but not complete the issue-1878 route.");
  expect(flex_engine.state().active->grass == 2 &&
             flex_engine.state().active->fire == 1,
         "The matchup-flex issue-1878 route did not complete GGF.");
  expect(std::find(flex_engine.state().discarded_this_turn.begin(),
                   flex_engine.state().discarded_this_turn.end(),
                   sim::Card::Dragapult) !=
             flex_engine.state().discarded_this_turn.end(),
         "The matchup-flex issue-1878 route did not create the current-turn payload.");
}
}  // namespace

int main() {
  try {
    test_issue_1878_uses_shared_current_turn_jit_semantics();
    std::cout << "Issue 2756 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
