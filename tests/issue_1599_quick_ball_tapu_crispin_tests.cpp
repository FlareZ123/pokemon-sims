
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {};
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
struct SeedResult { sim::TrialOutcome outcome; sim::TraceLog trace; };
SeedResult run_seed(const std::string& scenario_label, const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1599 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}
void test_seed_24_finishes_on_turn_two() {
  const SeedResult result = run_seed("matchup-flex-jit/go-second", 24);
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1599
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Seed 24 did not reach matchup-flex readiness on turn two.");
  expect(trace_contains(result.trace,
                        "Goodra VSTAR (Quick Ball issue-1599 Tapu-Crispin route cost)") &&
             trace_contains(result.trace, "T2 | WONDER TAG") &&
             trace_contains(result.trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(result.trace, "T2 | READY"),
         "Seed 24 did not use the complete Quick Ball-Tapu-Crispin route.");
}
void test_strict_seed_43_preserves_existing_turn_two_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 43);
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Strict seed 43 lost its existing turn-two route.");
}
void test_no_control_seed_42_preserves_quick_ball_hold() {
  const SeedResult result = run_seed("no-discard-control/go-first", 42);
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "No-control seed 42 lost its resource-preserving turn-three route.");
  expect(trace_contains(result.trace, "HOLD QUICK BALL"),
         "No-control seed 42 no longer preserves its redundant Quick Ball.");
}
}  // namespace
int main() {
  test_seed_24_finishes_on_turn_two();
  test_strict_seed_43_preserves_existing_turn_two_route();
  test_no_control_seed_42_preserves_quick_ball_hold();
  return 0;
}
