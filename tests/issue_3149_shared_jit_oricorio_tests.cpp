#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <optional>
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
  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.choose_discard(false, true, true, Card::QuickBall, false);
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool bench_oricorio(Engine& engine) {
    return engine.bench_oricorio_if_useful();
  }
  static bool hold_supporter(const Engine& engine) {
    return engine.issue_1419_oricorio_route_should_hold_supporter();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::LatiasEx,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Oricorio, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::GoodraVstar, sim::Card::Fire};
  state.prizes = {sim::Card::Arven, sim::Card::Gladion,
                  sim::Card::ForestSealStone, sim::Card::Lusamine,
                  sim::Card::RegidragoV, sim::Card::EarthenVessel};
  return state;
}

void shared_jit_profile(const sim::DciProfile dci, const char* label) {
  const sim::Scenario scenario{"issue-3149", dci, sim::LockMode::None, true, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3149);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // Quick Ball's printed cost/search and the Oricorio route are physically the
  // same under both same-turn-JIT profiles. Latias ex is route-inert once the
  // modeled Regidrago VSTAR is already Active, and Blender supplies the payload
  // on the eventual ready turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Shared JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3149
  if (sim::EngineTestAccess::quick_ball_cost(engine) != sim::Card::LatiasEx) {
    throw std::runtime_error(std::string(label) +
                             ": shared-JIT route did not expose Latias ex");
  }
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball did not resolve the shared-JIT Oricorio search");
  expect(sim::EngineTestAccess::bench_oricorio(engine),
         "Oricorio did not resolve Vital Dance");
  if (!sim::EngineTestAccess::hold_supporter(engine)) {
    throw std::runtime_error(std::string(label) +
                             ": post-Vital-Dance route did not protect the hand");
  }
}

void test_both_same_turn_jit_profiles() {
  shared_jit_profile(sim::DciProfile::StrictJit, "StrictJit");
  shared_jit_profile(sim::DciProfile::MatchupFlexJit, "MatchupFlexJit");
}

void test_no_discard_control_stays_outside_route_fallback() {
  const sim::Scenario scenario{"issue-3149-ndc", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3150);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());
  expect(sim::EngineTestAccess::quick_ball_cost(engine) != sim::Card::LatiasEx,
         "NoDiscardControl incorrectly entered the same-turn-JIT fallback");
}

void test_matchup_flex_keeps_lower_dci_cost_first() {
  const sim::Scenario scenario{"issue-3149-flex", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3151);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = route_state();
  state.hand.push_back(sim::Card::Dipplin);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  expect(sim::EngineTestAccess::quick_ball_cost(engine) == sim::Card::Dipplin,
         "route-specific Latias fallback displaced lower-DCI fodder");
}

}  // namespace

int main() {
  test_both_same_turn_jit_profiles();
  test_no_discard_control_stays_outside_route_fallback();
  test_matchup_flex_keeps_lower_dci_cost_first();
  return 0;
}
