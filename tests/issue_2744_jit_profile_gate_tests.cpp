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
    return engine.issue_1875_quick_ball_tapu_crispin_route_available();
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

sim::Engine make_engine(const sim::DciProfile profile, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Scenario scenario{"issue-2744", profile, sim::LockMode::None, true, 5};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());
  return engine;
}

void test_route_uses_shared_jit_timing_semantics() {
  std::mt19937_64 rng(2744);
  sim::Engine strict = make_engine(sim::DciProfile::StrictJit, rng);
  sim::Engine flex = make_engine(sim::DciProfile::MatchupFlexJit, rng);
  sim::Engine control = make_engine(sim::DciProfile::NoDiscardControl, rng);

  // Strict JIT and matchup-flex JIT share same-ready-turn payload timing. The
  // no-discard-control profile may bank payloads earlier and stays outside this JIT route.
  // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(sim::EngineTestAccess::route_available(strict),
         "Strict JIT lost the issue-1875 Tapu-Crispin route.");
  expect(sim::EngineTestAccess::route_available(flex),
         "Matchup-flex JIT still rejects the issue-1875 Tapu-Crispin route.");
  expect(!sim::EngineTestAccess::route_available(control),
         "No-discard-control incorrectly entered the JIT-specific Tapu-Crispin route.");
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_seed_157_flex_reaches_t3_through_tapu_crispin() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-2744 registered fixture unavailable.");

  std::mt19937_64 rng(157);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Quick Ball supplies the ready-turn Dragon discard and Tapu Lele-GX converts
  // the remaining Supporter axis into Crispin for the final Grass attachment.
  // Earliest route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/2744
  expect(outcome.first_ready_turn == 3,
         "Seed 157 matchup-flex JIT did not reach T3 after profile generalization.");
  expect(trace_contains(trace, "Quick Ball issue-1875 Tapu-Crispin route cost") &&
             trace_contains(trace, "WONDER TAG") && trace_contains(trace, "Crispin"),
         "Seed 157 did not use the issue-1875 Tapu-Crispin route.");
}
}  // namespace

int main() {
  try {
    test_route_uses_shared_jit_timing_semantics();
    test_seed_157_flex_reaches_t3_through_tapu_crispin();
    std::cout << "Issue 2744 JIT profile gate tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
