#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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
    return engine.issue_1874_duplicate_treasure_payload_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1874_duplicate_treasure_payload_route();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0};
  state.hand = {sim::Card::Fire, sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure, sim::Card::EarthenVessel};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  return state;
}

void test_issue_1874_uses_shared_current_turn_jit_semantics() {
  std::mt19937_64 rng(2759);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict{"issue-2759-strict", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  const sim::Scenario flex{"issue-2759-flex", sim::DciProfile::MatchupFlexJit,
                           sim::LockMode::None, true, 5};
  const sim::Scenario control{"issue-2759-control", sim::DciProfile::NoDiscardControl,
                              sim::LockMode::None, true, 5};
  sim::Engine strict_engine(strict, recipe, rng);
  sim::Engine flex_engine(flex, recipe, rng);
  sim::Engine control_engine(control, recipe, rng);
  sim::EngineTestAccess::set_state(strict_engine, route_state());
  sim::EngineTestAccess::set_state(flex_engine, route_state());
  sim::EngineTestAccess::set_state(control_engine, route_state());

  // Both JIT profiles require the searched Dragon to enter discard on this same
  // ready turn. NoDiscardControl permits earlier banking and stays outside the route.
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2759
  expect(sim::EngineTestAccess::route_available(strict_engine),
         "StrictJit lost the issue-1874 duplicate-Treasure route.");
  expect(sim::EngineTestAccess::route_available(flex_engine),
         "MatchupFlexJit still rejects the issue-1874 duplicate-Treasure route.");
  expect(!sim::EngineTestAccess::route_available(control_engine),
         "NoDiscardControl incorrectly entered the JIT-specific issue-1874 route.");
  expect(sim::EngineTestAccess::complete_route(flex_engine),
         "MatchupFlexJit could detect but not complete the issue-1874 route.");
}
}  // namespace

int main() {
  try {
    test_issue_1874_uses_shared_current_turn_jit_semantics();
    std::cout << "Issue 2759 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
