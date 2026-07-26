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

void test_seed_269_recovers_and_executes_the_full_payload_bridge() {
  const auto scenario =
      sim::scenario_by_label("no-discard-control/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1576 fixture is unavailable.");

  // Keep the source-bound reproduction seed exact: https://github.com/FlareZ123/pokemon-sims/issues/1576
  std::mt19937_64 rng{269};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Legacy Star recovers Mysterious Treasure and Quick Ball. Earthen Vessel
  // spends the now-inert Steven's Resolve, searches two Grass Energy, Treasure
  // spends one Grass while preserving the other attachment, and Quick Ball
  // discards the searched Dragon payload for the T2 ready state:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Core Item, Supporter, search, discard, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1576
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 269 did not reach no-discard-control readiness on turn two.");
  expect(trace_contains(trace,
                        "recovered: Mysterious Treasure, Quick Ball") &&
             trace_contains(trace,
                            "Steven's Resolve (Earthen Vessel cost)") &&
             trace_contains(trace,
                            "Grass Energy (Mysterious Treasure cost)") &&
             trace_contains(trace,
                            "Mega Dragonite ex (Quick Ball cost)") &&
             trace_contains(trace, "T2 | READY |"),
         "Seed 269 did not execute the complete Legacy Star payload bridge.");
}
}  // namespace

int main() {
  try {
    test_seed_269_recovers_and_executes_the_full_payload_bridge();
    std::cout << "Issue 1576 Legacy Star payload bridge tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
