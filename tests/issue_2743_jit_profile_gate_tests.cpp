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
    return engine.issue_1877_treasure_quick_ball_payload_bridge_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1877_treasure_quick_ball_payload_bridge();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::FieldBlower};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV};
  return state;
}

void test_route_uses_shared_jit_timing_semantics() {
  std::mt19937_64 rng(2743);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict_scenario{
      "issue-2743-strict", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  const sim::Scenario flex_scenario{
      "issue-2743-flex", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::None, true, 5};
  const sim::Scenario control_scenario{
      "issue-2743-control", sim::DciProfile::NoDiscardControl,
      sim::LockMode::None, true, 5};
  sim::Engine strict(strict_scenario, recipe, rng);
  sim::Engine flex(flex_scenario, recipe, rng);
  sim::Engine control(control_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(strict, route_state());
  sim::EngineTestAccess::set_state(flex, route_state());
  sim::EngineTestAccess::set_state(control, route_state());

  // Strict JIT and matchup-flex JIT share same-ready-turn payload timing.
  // No-discard-control permits earlier payload banking and stays outside this route.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2743
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1877 bridge.");
  expect(sim::EngineTestAccess::route_available(flex),
         "Matchup-flex JIT still rejects the issue-1877 bridge.");
  expect(!sim::EngineTestAccess::route_available(control),
         "No-discard-control incorrectly entered the JIT-specific bridge.");

  // The exact-state fixture also executes the full flex route, proving that the
  // searched Dragon enters discard on turn 3 and immediately satisfies Apex Dragon.
  // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(sim::EngineTestAccess::complete_route(flex),
         "Matchup-flex JIT could detect but not complete the issue-1877 bridge.");
}
}  // namespace

int main() {
  try {
    test_route_uses_shared_jit_timing_semantics();
    std::cout << "Issue 2743 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
