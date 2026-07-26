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
bool contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&](const std::string& line) { return line.find(needle) != std::string::npos; });
}
void test_seed_17_uses_zero_search_for_promotion() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr, "Issue-1561 fixture unavailable");
  std::mt19937_64 rng{17};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  // Exploding Energy searches for up to five cards and self-KOs when the deck was
  // searched in this way, so a legal zero-card search still performs the self-KO:
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Knock Out and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1561
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed, "Seed 17 missed T3 readiness");
  expect(contains(trace, "Exploding Energy attached 0 Basic Grass Energy") && contains(trace, "T3 | PROMOTE") && contains(trace, "T3 | READY"), "Seed 17 missed the zero-search promotion route");
}
}
int main() { try { test_seed_17_uses_zero_search_for_promotion(); std::cout << "Issue 1561 tests passed\n"; return 0; } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; } }
