
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(const bool c, const char* m) { if (!c) throw std::runtime_error(m); }
bool trace_contains(const sim::TraceLog& t, const std::string& x) { return std::any_of(t.lines.begin(), t.lines.end(), [&x](const std::string& l){ return l.find(x) != std::string::npos; }); }
struct SeedResult { sim::TrialOutcome outcome; sim::TraceLog trace; };
SeedResult run_seed(const std::string& deck_id, const std::string& scenario_label, const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label); const sim::NamedDeck* deck = sim::deck_by_id(deck_id);
  expect(scenario.has_value() && deck != nullptr, "The issue-1596 fixture is unavailable.");
  std::mt19937_64 rng(seed); sim::TraceLog trace{true, {}}; sim::Engine engine(*scenario, deck->recipe, rng, &trace); return {engine.run(), std::move(trace)};
}
void test_seed_26_plays_turo_before_vessel() {
  const SeedResult result = run_seed("regidrago-pineco", "matchup-flex-jit/go-second", 26);
  // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1596
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed, "Pineco seed 26 did not reach readiness on turn three.");
  expect(trace_contains(result.trace, "T3 | PLAY SUPPORTER") && trace_contains(result.trace, "Professor Turo returned Active Dialga-GX") && trace_contains(result.trace, "Dialga-GX (Earthen Vessel cost)") && trace_contains(result.trace, "T3 | READY"), "Seed 26 did not preserve the Turo-before-Vessel route.");
}
void test_pineco_seed_35_keeps_existing_t2() { const SeedResult r = run_seed("regidrago-pineco", "strict-jit/go-second", 35); expect(r.outcome.first_ready_turn == 2 && !r.outcome.setup_failed, "Pineco seed 35 lost its existing T2 route."); }
void test_shell_seed_43_keeps_existing_t2() { const SeedResult r = run_seed("regidrago-shell", "strict-jit/go-first", 43); expect(r.outcome.first_ready_turn == 2 && !r.outcome.setup_failed, "Shell seed 43 lost its existing T2 route."); }
}
int main() { test_seed_26_plays_turo_before_vessel(); test_pineco_seed_35_keeps_existing_t2(); test_shell_seed_43_keeps_existing_t2(); return 0; }
