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
         "The registered issue-1514 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_seed_33_searches_first_and_reaches_t3() {
  const SeedResult result =
      run_seed("no-discard-control/go-second", 33);

  // A legal Quick Ball or Mysterious Treasure search establishes K1 before any
  // Prize-aware choice. The held Crispin already covers the Energy Supporter,
  // so early Wonder Tag must preserve its one-shot Tapu and Bench resources:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
  expect(result.outcome.first_ready_turn == 3 &&
             !result.outcome.setup_failed,
         "Seed 33 did not improve from T4 to the legal T3 route.");
  expect(trace_contains(result.trace, "T1 | HOLD TAPU LELE-GX"),
         "Seed 33 did not preserve Tapu before the legal search-first route.");
  expect(!trace_contains(result.trace,
                         "T1 | BENCH | Tapu Lele-GX from hand.") &&
             !trace_contains(result.trace, "T1 | WONDER TAG"),
         "Seed 33 still spent Tapu on the duplicate T1 Crispin.");
  expect(trace_contains(result.trace, "T3 | READY"),
         "Seed 33 did not finish the preserved-Tapu T3 route.");
}

void test_seed_104_keeps_distinct_tapu_crispin_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 104);

  // This control has no duplicate held Crispin. Quick Ball into Tapu Lele-GX
  // remains the live connector that supplies Crispin before Earthen Vessel:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/962
  expect(result.outcome.first_ready_turn == 3 &&
             !result.outcome.setup_failed,
         "The distinct seed-104 Tapu route lost its T3 deadline.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin"),
         "The distinct seed-104 Tapu-Crispin connector was suppressed.");
}

void test_seed_43_keeps_t2_treasure_tapu_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 43);

  // Mysterious Treasure supplies the distinct Tapu-to-Crispin connector after
  // preserving the Supporter action. This established T2 line must remain live:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(result.outcome.first_ready_turn == 2 &&
             !result.outcome.setup_failed,
         "The distinct seed-43 connector lost its T2 deadline.");
  expect(trace_contains(result.trace, "T2 | HOLD SUPPORTER") &&
             trace_contains(result.trace, "T2 | WONDER TAG") &&
             trace_contains(result.trace, "T2 | READY"),
         "The distinct seed-43 Treasure-Tapu route was suppressed.");
}

}  // namespace

int main() {
  test_seed_33_searches_first_and_reaches_t3();
  test_seed_104_keeps_distinct_tapu_crispin_route();
  test_seed_43_keeps_t2_treasure_tapu_route();
  return 0;
}
