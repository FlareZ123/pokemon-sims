#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool advance(Engine& engine) { return engine.advance_forretress_combo(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State state_with_pineco(const int entered_turn) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(
      sim::Pokemon{sim::Card::Pineco, entered_turn, 0, 0, sim::Tool::None});
  state.hand = {sim::Card::ForestOfVitality, sim::Card::ForretressEx};
  return state;
}

void test_seed1_preserves_forest_and_t2_readiness() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-1 fixture is unavailable.");

  std::mt19937_64 rng(1);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Setup Pineco is prior-turn eligible on T2, so Forest changes no evolution
  // legality and must remain available while the route keeps the same deadline:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1377
  expect(outcome.first_ready_turn == 2,
         "The corrected seed-1 route lost strict-JIT T2 readiness.");
  expect(trace_contains(trace, "under normal prior-turn timing"),
         "The seed-1 Pineco did not use ordinary evolution timing.");
  expect(!trace_contains(trace, "T2 | PLAY STADIUM | rules: R-FOREST-VITALITY-01"),
         "The seed-1 route still played inert Forest of Vitality.");
}

void test_prior_turn_pineco_uses_ordinary_evolution() {
  const sim::Scenario scenario{"issue-1377-prior-turn",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(137701);
  sim::TraceLog trace{true, {}};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, state_with_pineco(1));

  expect(sim::EngineTestAccess::advance(engine),
         "The prior-turn Forretress route did not advance.");
  expect(sim::EngineTestAccess::state(engine).stadium == sim::Stadium::None,
         "Prior-turn Pineco consumed Forest of Vitality.");
  expect(std::count(sim::EngineTestAccess::state(engine).hand.begin(),
                    sim::EngineTestAccess::state(engine).hand.end(),
                    sim::Card::ForestOfVitality) == 1,
         "Prior-turn Pineco did not preserve Forest in hand.");
  expect(trace_contains(trace, "under normal prior-turn timing"),
         "Prior-turn Pineco failed ordinary evolution.");
}

void test_current_turn_pineco_keeps_forest_route() {
  const sim::Scenario scenario{"issue-1377-current-turn",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(137702);
  sim::TraceLog trace{true, {}};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, state_with_pineco(2));

  // Forest remains necessary for a Pineco played this turn after the first turn:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1377
  expect(sim::EngineTestAccess::advance(engine),
         "The current-turn Forest route did not advance.");
  expect(sim::EngineTestAccess::state(engine).stadium ==
             sim::Stadium::ForestOfVitality,
         "Current-turn Pineco lost its legal Forest route.");
  expect(trace_contains(trace, "Forest of Vitality allowed"),
         "Current-turn Pineco did not use Forest-enabled evolution.");
}

void test_missing_forretress_preserves_forest() {
  const sim::Scenario scenario{"issue-1377-no-forretress",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng(137703);
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = state_with_pineco(2);
  state.hand = {sim::Card::ForestOfVitality};
  state.deck.clear();
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  expect(!sim::EngineTestAccess::advance(engine),
         "A target-dead Forretress route changed state.");
  expect(sim::EngineTestAccess::state(engine).stadium == sim::Stadium::None,
         "Forest was played without a known Forretress target.");
}

}  // namespace

int main() {
  try {
    test_seed1_preserves_forest_and_t2_readiness();
    test_prior_turn_pineco_uses_ordinary_evolution();
    test_current_turn_pineco_keeps_forest_route();
    test_missing_forretress_preserves_forest();
    std::cout << "Issue 1377 prior-turn Pineco Forest tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
