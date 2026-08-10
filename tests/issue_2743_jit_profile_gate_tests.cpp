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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1877_treasure_quick_ball_payload_bridge_available();
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

sim::Engine make_engine(const sim::DciProfile profile, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Scenario scenario{"issue-2743", profile, sim::LockMode::None, true, 5};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());
  return engine;
}

void test_route_uses_shared_jit_timing_semantics() {
  std::mt19937_64 rng(2743);
  sim::Engine strict = make_engine(sim::DciProfile::StrictJit, rng);
  sim::Engine flex = make_engine(sim::DciProfile::MatchupFlexJit, rng);
  sim::Engine control = make_engine(sim::DciProfile::NoDiscardControl, rng);

  // Strict JIT and matchup-flex JIT share same-ready-turn payload timing, while
  // no-discard-control permits earlier banking and remains outside this JIT route.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2743
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1877 bridge.");
  expect(sim::EngineTestAccess::route_available(flex),
         "Matchup-flex JIT still rejects the issue-1877 bridge.");
  expect(!sim::EngineTestAccess::route_available(control),
         "No-discard-control incorrectly entered the JIT-specific bridge.");
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_seed_2194_flex_reaches_t3_through_bridge() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-2743 registered fixture unavailable.");

  std::mt19937_64 rng(2194);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // This exact K1 route discards route-replaced Earthen Vessel to Treasure,
  // then discards the searched Dragon to Quick Ball on the ready turn.
  // Route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2743
  expect(outcome.first_ready_turn == 3,
         "Seed 2194 matchup-flex JIT did not reach T3 after profile generalization.");
  expect(trace_contains(trace, "Mysterious Treasure spent route-replaced Earthen Vessel") &&
             trace_contains(trace, "Quick Ball discarded the searched Dragon payload"),
         "Seed 2194 did not use the issue-1877 payload bridge.");
}
}  // namespace

int main() {
  try {
    test_route_uses_shared_jit_timing_semantics();
    test_seed_2194_flex_reaches_t3_through_bridge();
    std::cout << "Issue 2743 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
