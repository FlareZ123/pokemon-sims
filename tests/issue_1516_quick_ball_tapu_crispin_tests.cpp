#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static const State& state(const Engine& engine) {
    return engine.state_;
  }
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

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_seed(const std::string& scenario_label,
                    const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1516 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_seed_42_preserves_quick_ball_and_tapu() {
  const SeedResult result = run_seed("no-discard-control/go-first", 42);

  // Earthen Vessel establishes K1 and loads Dragapult ex. Held Gladion covers the
  // known prized Mega Dragonite ex, while held Crispin remains the Energy Supporter.
  // Quick Ball into Tapu and duplicate Crispin therefore spends physical resources
  // without improving the T3 deadline:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // No-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Seed 42 lost its legal T3 ready turn.");
  expect(trace_contains(result.trace, "T1 | HOLD QUICK BALL"),
         "Seed 42 did not preserve the redundant Quick Ball route.");
  expect(!trace_contains(result.trace, "T1 | PLAY ITEM") &&
             !trace_contains(result.trace,
                             "T1 | BENCH | rules: R-GAME-BENCH | Tapu Lele-GX") &&
             !trace_contains(result.trace, "T1 | WONDER TAG"),
         "Seed 42 still spent Quick Ball and Tapu on duplicate Crispin.");
  expect(trace_contains(result.trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(result.trace, "Gladion") &&
             trace_contains(result.trace, "T3 | READY"),
         "Seed 42 did not retain the Gladion-to-Crispin T3 continuation.");
}

void test_strict_jit_seed_104_keeps_distinct_tapu_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 104);

  // Strict JIT needs the distinct Tapu Lele-GX to Crispin connector, so the
  // no-control payload-Prize guard must remain inactive:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/962
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Strict-JIT seed 104 lost its T3 route.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin"),
         "Strict-JIT seed 104 lost its distinct Tapu-Crispin connector.");
}

}  // namespace

int main() {
  test_seed_42_preserves_quick_ball_and_tapu();
  test_strict_jit_seed_104_keeps_distinct_tapu_route();
  return 0;
}
