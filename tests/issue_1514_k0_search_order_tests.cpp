#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
bool contains(const sim::TraceLog& trace, const std::string& needle) { return std::any_of(trace.lines.begin(), trace.lines.end(), [&](const std::string& line) { return line.find(needle) != std::string::npos; }); }
sim::TrialOutcome run(const std::string& label, std::uint64_t seed, sim::TraceLog& trace) {
  const auto scenario = sim::scenario_by_label(label); const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr, "Issue-1514 fixture unavailable");
  std::mt19937_64 rng{seed}; sim::Engine engine(*scenario, deck->recipe, rng, &trace); return engine.run();
}
void test_seed_33_searches_before_duplicate_crispin_wonder_tag() {
  sim::TraceLog trace{true, {}}; const auto outcome = run("no-discard-control/go-second", 33, trace);
  // The physical Quick Ball search establishes K1 before Wonder Tag is evaluated;
  // no hidden deck or Prize identity is consulted during the K0 hold decision.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Core search and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed, "Seed 33 missed T3 readiness");
  expect(contains(trace, "T1 | HOLD TAPU LELE-GX") && contains(trace, "T1 | PLAY ITEM") && contains(trace, "Quick Ball") && contains(trace, "T3 | READY"), "Seed 33 missed public search-first route");
  expect(!contains(trace, "T1 | WONDER TAG"), "Seed 33 still spent T1 Wonder Tag on duplicate Crispin");
}
}
int main() { try { test_seed_33_searches_before_duplicate_crispin_wonder_tag(); std::cout << "Issue 1514 tests passed\n"; return 0; } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; } }
