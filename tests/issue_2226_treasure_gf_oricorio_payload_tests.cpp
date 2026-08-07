#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_strict_seed_475_reaches_t4() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2226 strict fixture is unavailable.");

  std::mt19937_64 rng(475);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T1 Quick Ball already established K1. T4 Arven finds this Treasure plus
  // Forest Seal Stone, Star Alchemy finds Regidrago VSTAR, and Treasure searches
  // Oricorio. Vital Dance supplies Grass for the unused manual attachment. The
  // Treasure cost can therefore spend Dragapult ex for the same-turn payload.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio GRI 55: https://api.pokemontcg.io/v2/cards/sm2-55
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Ability, VSTAR Power, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2226
  expect(outcome.first_ready_turn == 4,
         "Strict seed 475 did not reach readiness on T4.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)"),
         "Strict seed 475 did not spend Dragapult ex as Treasure's cost.");
  expect(!trace_contains(trace,
                         "Mysterious Treasure (Mysterious Treasure cost)"),
         "Strict seed 475 still spent the duplicate Treasure.");
}

void test_matchup_flex_seed_475_keeps_t4() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2226 matchup-flex fixture is unavailable.");
  std::mt19937_64 rng(475);
  sim::Engine engine(*scenario, deck->recipe, rng);
  expect(engine.run().first_ready_turn == 4,
         "Issue 2226 regressed the existing matchup-flex T4 finish.");
}
}  // namespace

int main() {
  try {
    test_strict_seed_475_reaches_t4();
    test_matchup_flex_seed_475_keeps_t4();
    std::cout << "Issue 2226 GF Treasure Oricorio payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}