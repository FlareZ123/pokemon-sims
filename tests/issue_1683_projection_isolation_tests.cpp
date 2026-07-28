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
  static bool dead_quick_ball(const Engine& engine) {
    return engine.issue_1683_rulebox_locked_quick_ball_is_serena_cost();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State live_payload_quick_ball_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1},
      sim::Pokemon{sim::Card::RegidragoV, 2},
  };
  state.hand = {
      sim::Card::Serena,
      sim::Card::QuickBall,
      sim::Card::MysteriousTreasure,
      sim::Card::Dipplin,
  };
  state.deck = {
      sim::Card::DialgaGX,
      sim::Card::MegaDragonite,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
  };
  state.prizes = {
      sim::Card::Gladion,
      sim::Card::ProfessorBurnet,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::RoseannesBackup,
      sim::Card::Fire,
  };
  return state;
}

void test_live_projection_preserves_rng_and_trace() {
  const sim::Scenario scenario{
      "issue-1683/projection-isolation", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  const sim::State state = live_payload_quick_ball_state();

  std::mt19937_64 route_rng{16830};
  sim::TraceLog route_trace{true, {}};
  sim::Engine route_engine(
      scenario, sim::baseline_recipe(), route_rng, &route_trace);
  sim::EngineTestAccess::set_state(route_engine, state);

  // Quick Ball can legally pay Dipplin, search the Basic Dragon Dialga-GX, and
  // preserve Mysterious Treasure as the later payload-discard outlet. This control
  // proves that the Serena classifier is projecting a route that shuffles the deck:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dipplin: https://api.pokemontcg.io/v2/cards/sv6-127
  // Official Item, discard, search, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // DCI and connector preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(sim::EngineTestAccess::play_quick_ball(route_engine),
         "The control state did not expose a live Quick Ball payload route.");
  expect(!route_trace.lines.empty(),
         "The control Quick Ball route did not emit its expected trace.");

  std::mt19937_64 rng{16831};
  std::mt19937_64 expected_rng = rng;
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, state);

  // Engine stores the trial RNG by reference. A copied projection therefore shares
  // that RNG unless the classifier explicitly restores it, and a copied trace pointer
  // would otherwise leak speculative actions into the real trace:
  // C++ reference-member copy semantics: https://eel.is/c++draft/class.copy.ctor#15
  // Repository fixed-seed sampling contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#sampling-and-comparison-method
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
  // Validated pull request: https://github.com/FlareZ123/pokemon-sims/pull/1689
  expect(!sim::EngineTestAccess::dead_quick_ball(engine),
         "A live Quick Ball route was incorrectly classified as setup-dead.");
  expect(trace.lines.empty(),
         "The silent Quick Ball projection leaked speculative trace lines.");
  expect(rng() == expected_rng(),
         "The silent Quick Ball projection advanced the live trial RNG.");
}

}  // namespace

int main() {
  try {
    test_live_projection_preserves_rng_and_trace();
    std::cout << "Issue 1683 projection isolation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
