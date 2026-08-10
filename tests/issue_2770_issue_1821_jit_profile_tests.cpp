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
  static bool route_available(const Engine& engine) {
    return engine.issue_1821_steven_latias_grass_route_available();
  }
  static bool play_route(Engine& engine) {
    return engine.play_issue_1821_steven_latias_grass_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::StevensResolve, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::Crispin, sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::RegidragoV, sim::Card::LatiasEx, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire, sim::Card::Fire,
                sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::ErikasInvitation, sim::Card::MysteriousTreasure,
                  sim::Card::Grass, sim::Card::Dipplin, sim::Card::Powerglass,
                  sim::Card::Lusamine};
  return state;
}

sim::Scenario scenario(const sim::DciProfile dci) {
  return sim::Scenario{"issue-2770", dci, sim::LockMode::None, false, 5};
}

void test_issue_1821_route_uses_shared_jit_timing() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario strict_scenario = scenario(sim::DciProfile::StrictJit);
  const sim::Scenario flex_scenario = scenario(sim::DciProfile::MatchupFlexJit);
  const sim::Scenario control_scenario = scenario(sim::DciProfile::NoDiscardControl);
  std::mt19937_64 strict_rng(2770), flex_rng(2770), control_rng(2770);
  sim::Engine strict_engine(strict_scenario, recipe, strict_rng);
  sim::Engine flex_engine(flex_scenario, recipe, flex_rng);
  sim::Engine control_engine(control_scenario, recipe, control_rng);
  sim::EngineTestAccess::set_state(strict_engine, route_state());
  sim::EngineTestAccess::set_state(flex_engine, route_state());
  sim::EngineTestAccess::set_state(control_engine, route_state());

  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original route: https://github.com/FlareZ123/pokemon-sims/issues/1821
  // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2770
  expect(sim::EngineTestAccess::route_available(strict_engine), "StrictJit lost issue-1821 route");
  expect(sim::EngineTestAccess::route_available(flex_engine), "MatchupFlexJit still rejects issue-1821 route");
  expect(!sim::EngineTestAccess::route_available(control_engine), "NoDiscardControl incorrectly entered same-turn JIT route");
  expect(sim::EngineTestAccess::play_route(flex_engine), "MatchupFlexJit could not execute issue-1821 route");
  const sim::State& after = sim::EngineTestAccess::state(flex_engine);
  expect(after.active && after.active->card == sim::Card::TapuLeleGX, "Tapu Lele-GX was not promoted before Steven");
  expect(after.manual_energy_used && after.retreat_used && after.supporter_used && after.turn_ended, "Issue-1821 T1 actions were not consumed");
  expect(contains(after.hand, sim::Card::RegidragoV) && contains(after.hand, sim::Card::LatiasEx) && contains(after.hand, sim::Card::Grass), "Steven did not bank the issue-1821 package");
}
}  // namespace

int main() {
  try {
    test_issue_1821_route_uses_shared_jit_timing();
    std::cout << "Issue 2770 shared-JIT route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
