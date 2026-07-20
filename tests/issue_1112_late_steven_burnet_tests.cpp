#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool late_steven_burnet_route(const Engine& engine) {
    return engine.late_steven_vstar_grass_with_held_burnet_route_available();
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

sim::State issue_1112_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::Dragapult};
  state.discard = {sim::Card::RegidragoVstar};
  return state;
}

sim::Scenario issue_1112_scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1112-late-steven-burnet",
                       sim::DciProfile::StrictJit, lock, true, 5};
}

void test_exact_k1_state_admits_route() {
  const sim::Scenario scenario = issue_1112_scenario();
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1112};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_k1_state(engine, issue_1112_state());

  // Steven searches VSTAR plus the final Grass. The prior-turn Active can attach,
  // evolve, and use held Burnet for the same-turn strict-JIT payload:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1112
  expect(sim::EngineTestAccess::late_steven_burnet_route(engine),
         "The exact K1 VSTAR-plus-Grass route with held Burnet must admit Steven.");
}

void test_required_components_gate_route() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    const sim::Scenario scenario = issue_1112_scenario();
    std::mt19937_64 rng{1113};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = issue_1112_state();
    std::erase(state.hand, sim::Card::ProfessorBurnet);
    sim::EngineTestAccess::set_k1_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::late_steven_burnet_route(engine),
           "Missing held Burnet must reject the route.");
  }

  {
    const sim::Scenario scenario = issue_1112_scenario();
    std::mt19937_64 rng{1114};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = issue_1112_state();
    std::erase(state.deck, sim::Card::Grass);
    std::erase(state.deck, sim::Card::Grass);
    sim::EngineTestAccess::set_k1_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::late_steven_burnet_route(engine),
           "K1-proven missing Grass must reject the route.");
  }

  {
    const sim::Scenario scenario =
        issue_1112_scenario(sim::LockMode::FullItem);
    std::mt19937_64 rng{1115};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_k1_state(engine, issue_1112_state());
    // The confirmed route is restricted to the repository no-lock scenario:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    expect(!sim::EngineTestAccess::late_steven_burnet_route(engine),
           "Full Item lock must reject the route.");
  }
}

void test_seed_493_uses_steven_and_reaches_turn_four() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{493};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The deterministic Steven line replaces T3 Celestial Roar. Serena may legally
  // discard a held Dragon on T4 before evolution, which is at least as early as the
  // held Burnet continuation and preserves the confirmed T4 ready turn:
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Repository priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1112
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 493 must play Steven on T3.");
  expect(!trace_contains(trace, "T3 | ATTACK | rules: R-RV-01"),
         "Seed 493 must not use Celestial Roar on T3.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 493 must reach strict-JIT readiness on T4.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
         "Seed 493 must first become ready on T4.");
}
}  // namespace

int main() {
  test_exact_k1_state_admits_route();
  test_required_components_gate_route();
  test_seed_493_uses_steven_and_reaches_turn_four();
  return 0;
}
