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
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1875_quick_ball_tapu_crispin_route();
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
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0};
  state.hand = {sim::Card::Fire, sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Crispin, sim::Card::TapuLeleGX};
  return state;
}

void test_route_uses_shared_jit_timing_semantics() {
  std::mt19937_64 rng(2744);
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict_scenario{
      "issue-2744-strict", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 5};
  const sim::Scenario flex_scenario{
      "issue-2744-flex", sim::DciProfile::MatchupFlexJit,
      sim::LockMode::None, true, 5};
  const sim::Scenario control_scenario{
      "issue-2744-control", sim::DciProfile::NoDiscardControl,
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
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Engine lifetime contract: https://github.com/FlareZ123/pokemon-sims/issues/869
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1875 Tapu-Crispin route.");
  expect(sim::EngineTestAccess::route_available(flex),
         "Matchup-flex JIT still rejects the issue-1875 Tapu-Crispin route.");
  expect(!sim::EngineTestAccess::route_available(control),
         "No-discard-control incorrectly entered the JIT-specific Tapu-Crispin route.");

  // Execute the complete matchup-flex route from the same K1 public state.
  // Quick Ball supplies the current-turn Dragon discard; Wonder Tag converts the
  // Bench action into Crispin; Crispin searches two different Basic Energy types
  // so one can be attached by its printed effect to complete Apex Dragon's GGF.
  // Earliest route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(sim::EngineTestAccess::complete_route(flex),
         "Matchup-flex JIT could detect but not complete the issue-1875 Tapu-Crispin route.");
}
}  // namespace

int main() {
  try {
    test_route_uses_shared_jit_timing_semantics();
    std::cout << "Issue 2744 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
