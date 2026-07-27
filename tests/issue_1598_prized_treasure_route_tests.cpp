#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

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

void test_seed_14_prefers_prized_treasure_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1598 fixture is unavailable.");

  std::mt19937_64 rng{14};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Gladion reveals Mysterious Treasure. The held Fire guarantees next-turn GGF,
  // and Treasure may discard Mega Dragonite ex while searching Regidrago VSTAR:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-166
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1598
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "Seed 14 did not reach strict-JIT readiness on turn three.");
  expect(trace_contains(trace, "exchanged Gladion for Mysterious Treasure") &&
             !trace_contains(trace, "T2 | DISCARD | rules: R-MT-01") &&
             trace_contains(trace, "T3 | DISCARD | rules: R-MT-01 | Mega Dragonite ex") &&
             trace_contains(trace, "T3 | EVOLVE") &&
             trace_contains(trace, "T3 | READY"),
         "Seed 14 did not execute the banked prized-Treasure route.");
}
}

int main() {
  try {
    test_seed_14_prefers_prized_treasure_route();
    std::cout << "Issue 1598 prized-Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
