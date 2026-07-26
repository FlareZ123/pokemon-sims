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

void test_seed_191_uses_prized_fss_oricorio_route() {
  const auto scenario =
      sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1578 fixture is unavailable.");

  // Keep the source-bound reproduction seed exact: https://github.com/FlareZ123/pokemon-sims/issues/1578
  std::mt19937_64 rng{191};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Gladion may exchange itself for the known prized Forest Seal Stone. Star
  // Alchemy then searches Oricorio, Vital Dance searches two Grass Energy for
  // consecutive manual attachments, and Blender supplies the T4 JIT payload:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Prize, Supporter, Tool, Ability, Bench, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1578
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 191 did not reach matchup-flex readiness on turn four.");
  expect(trace_contains(trace, "Exchanged Gladion for Forest Seal Stone") &&
             trace_contains(trace, "Searched any card: Oricorio") &&
             trace_contains(trace, "Vital Dance") &&
             trace_contains(trace, "T4 | READY |"),
         "Seed 191 did not execute the prized FSS-Oricorio route.");
}
}  // namespace

int main() {
  try {
    test_seed_191_uses_prized_fss_oricorio_route();
    std::cout << "Issue 1578 Gladion-FSS-Oricorio tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
