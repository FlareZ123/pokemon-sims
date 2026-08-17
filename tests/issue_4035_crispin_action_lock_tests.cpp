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
    engine.prizes_revealed_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1393_held_crispin_completion_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None},
  };
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::RegidragoVstar,
      sim::Card::Crispin,
      sim::Card::Gladion,
      sim::Card::MegaDragonite,
      sim::Card::BrilliantBlender,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Oricorio,
      sim::Card::Dragapult,
      sim::Card::DialgaGX,
  };
  state.prizes = {sim::Card::GoodraVstar};
  return state;
}

bool route_available_under(const sim::LockMode locks) {
  const sim::Scenario scenario{"issue-4035", sim::DciProfile::StrictJit,
                               locks, false, 4};
  std::mt19937_64 rng{4035};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, exact_state());
  return sim::EngineTestAccess::route_available(engine);
}

void test_action_specific_locks() {
  // Crispin is the route's Supporter and Brilliant Blender is its Item payload
  // outlet. Path-style Rule Box Ability suppression does not prohibit either
  // Trainer action. Item or Supporter restrictions block a required action class.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Advanced Item/Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4035
  expect(route_available_under(sim::LockMode::None),
         "Unlocked held-Crispin route must stay available.");
  expect(route_available_under(sim::LockMode::FullRuleBoxAbility),
         "Rule Box Ability suppression must not block the Trainer-only route.");
  expect(!route_available_under(sim::LockMode::TurnTwoItem),
         "Turn-two Item lock must block the payload outlet.");
  expect(!route_available_under(sim::LockMode::FullItem),
         "Full Item lock must block the payload outlet.");
  expect(!route_available_under(sim::LockMode::FullSupporter),
         "Supporter lock must block Crispin.");
  expect(!route_available_under(sim::LockMode::FullCombined),
         "Combined lock must block the turn-two Item outlet.");
}
}  // namespace

int main() {
  try {
    test_action_specific_locks();
    std::cout << "Issue 4035 Crispin action-lock tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
