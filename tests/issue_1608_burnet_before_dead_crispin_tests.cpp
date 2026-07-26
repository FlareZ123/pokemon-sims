#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim { struct EngineTestAccess {}; }

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

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_crobat_seed(const std::string& variant_id,
                           const std::string& scenario_label,
                           const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::CrobatModelingDeck* deck =
      sim::crobat_modeling_deck_by_id(variant_id);
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1608 Crobat fixture is unavailable.");
  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

SeedResult run_shell_seed(const std::string& scenario_label,
                          const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-971 control fixture is unavailable.");
  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_seed_33_uses_burnet_before_dead_crispin() {
  const SeedResult result = run_crobat_seed(
      "crobat2-erika-channeler", "no-discard-control/go-first", 33);

  // GGF is already complete, so Crispin has zero immediate Energy-axis value.
  // Professor Burnet legally searches and discards the two deck payloads, then
  // shuffles. The regression asserts the observable T3 choice and never assumes
  // a particular post-shuffle T4 draw:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Core Supporter, search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1608
  expect(trace_contains(result.trace,
                        "Held Gladion because prized Crispin cannot improve complete GGF") &&
             trace_contains(result.trace,
                            "T3 | PLAY SUPPORTER | rules: R-BURNET-01") &&
             trace_contains(result.trace, "Mega Dragonite ex") &&
             trace_contains(result.trace, "Dragapult ex") &&
             !trace_contains(result.trace,
                             "T3 | PLAY SUPPORTER | rules: R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01 | Looked at Prize cards and exchanged Gladion for Crispin"),
         "Crobat seed 33 did not prefer observable Burnet payload progress over dead Crispin.");
}

void test_live_prized_crispin_route_remains_available() {
  const SeedResult result = run_shell_seed("strict-jit/go-second", 28);
  // Crispin remains live when Energy is unresolved, preserving completed #971:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Prior regression: https://github.com/FlareZ123/pokemon-sims/issues/971
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "The live prized-Crispin seed lost its turn-two route.");
  expect(trace_contains(result.trace, "exchanged Gladion for Crispin"),
         "The live prized-Crispin control no longer selects Crispin.");
}
}

int main() {
  test_seed_33_uses_burnet_before_dead_crispin();
  test_live_prized_crispin_route_remains_available();
  return 0;
}
