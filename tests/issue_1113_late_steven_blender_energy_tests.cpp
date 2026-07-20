#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static bool late_steven_blender_energy_route(const Engine& engine) {
    return engine.late_steven_vstar_grass_with_held_blender_k0_route_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State issue_1113_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::GoodraVstar};
  return state;
}

sim::Scenario issue_1113_scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1113-late-steven-blender-energy",
                       sim::DciProfile::StrictJit, lock, true, 5};
}

void test_exact_k0_state_admits_route() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1113};
  sim::Engine engine(issue_1113_scenario(), recipe, rng);
  sim::EngineTestAccess::set_state(engine, issue_1113_state(), false);

  // K0 keeps fixed-list VSTAR and Grass copies plausible. Steven's search can
  // reserve both, then the prior-turn Active attaches, evolves, and uses held
  // Blender for the next strict-JIT ready turn:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1113
  expect(sim::EngineTestAccess::late_steven_blender_energy_route(engine),
         "The exact K0 VSTAR-plus-Grass route with held Blender must admit Steven.");
}

void test_observable_blockers_reject_route() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    const sim::Scenario scenario = issue_1113_scenario();
    std::mt19937_64 rng{1114};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = issue_1113_state();
    std::erase(state.hand, sim::Card::BrilliantBlender);
    sim::EngineTestAccess::set_state(engine, std::move(state), false);
    expect(!sim::EngineTestAccess::late_steven_blender_energy_route(engine),
           "Missing held Blender must reject the route.");
  }

  {
    const sim::Scenario scenario = issue_1113_scenario();
    std::mt19937_64 rng{1115};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, issue_1113_state(), true);
    expect(!sim::EngineTestAccess::late_steven_blender_energy_route(engine),
           "The K0-only route must stop after legal deck inspection.");
  }

  {
    const sim::Scenario scenario =
        issue_1113_scenario(sim::LockMode::FullItem);
    std::mt19937_64 rng{1116};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, issue_1113_state(), false);
    // Blender is an Item, so the repository full-Item-lock scenario blocks it:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    expect(!sim::EngineTestAccess::late_steven_blender_energy_route(engine),
           "Full Item lock must reject the held-Blender route.");
  }
}

void test_seed_63_uses_steven_and_reaches_turn_three() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{63};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The complete K0 Steven route must outrank the low-information Celestial Roar.
  // A downstream held-payload discard route may improve on Blender while preserving
  // the required T3 deadline, so the regression asserts the admission and outcome:
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1113
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 63 must play Steven on T2.");
  expect(!trace_contains(trace, "T2 | ATTACK | rules: R-RV-01"),
         "Seed 63 must not use Celestial Roar on T2.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 63 must reach strict-JIT readiness on T3.");
  expect(outcome.ready_by_3 && outcome.first_ready_turn == 3,
         "Seed 63 must first become ready on T3.");
}
}  // namespace

int main() {
  test_exact_k0_state_admits_route();
  test_observable_blockers_reject_route();
  test_seed_63_uses_steven_and_reaches_turn_three();
  return 0;
}
