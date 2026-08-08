#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  for (const std::string& line : trace.lines) {
    if (line.find(text) != std::string::npos) return true;
  }
  return false;
}

sim::TraceLog run_trace(const char* scenario_label, const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2265 fixture is unavailable.");

  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();
  return trace;
}

void test_strict_seed_122_projects_arven_into_t3_quick_ball_finish() {
  const sim::TraceLog trace = run_trace("strict-jit/go-second", 122);

  // The existing T3 setup sequence reaches an evolution-eligible Regidrago V at
  // GGF while a second Arven, surplus Grass, and Brilliant Blender remain held.
  // Arven can then search the K1-known Quick Ball. The held Grass may pay Quick
  // Ball because Energy is complete; Latias ex supplies Skyliner and Blender
  // supplies the required strict-JIT Dragon payload before the free promotion.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Item, discard, Bench, evolution, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "Arven searched Quick Ball for the final surplus-Energy Latias/Blender route."),
         "Strict seed 122 did not project Arven into the Quick Ball continuation on T3.");
  expect(trace_contains(trace, "T3 | DISCARD") &&
             trace_contains(trace, "Grass Energy (Quick Ball cost)"),
         "Strict seed 122 did not spend the route-surplus Grass through Quick Ball on T3.");
  expect(trace_contains(trace, "T3 | READY"),
         "Strict seed 122 did not reach the legal T3 Arven/Quick Ball/Latias/Blender finish.");
}

void test_matchup_flex_seed_122_keeps_existing_t3_finish() {
  const sim::TraceLog trace = run_trace("matchup-flex-jit/go-second", 122);

  // The new Arven projection must preserve the established T3 matchup-flex witness.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined bug scope: https://github.com/FlareZ123/pokemon-sims/issues/2265#issuecomment-5215563905
  expect(trace_contains(trace, "T3 | READY"),
         "Matchup-flex seed 122 regressed from its established T3 finish.");
}
}  // namespace

int main() {
  try {
    test_strict_seed_122_projects_arven_into_t3_quick_ball_finish();
    test_matchup_flex_seed_122_keeps_existing_t3_finish();
    std::cout << "Issue 2265 Arven Quick Ball projection tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
