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

void test_seed_7950_strict_reaches_t4_with_payload_cost() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2232 strict fixture is unavailable.");

  std::mt19937_64 rng(7950);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The existing T4 line already searches Tapu Lele-GX with Mysterious Treasure,
  // Wonder Tags for Crispin, attaches one missing Energy with Crispin, and uses
  // the manual attachment for the other. Spending Mega Dragonite ex as Treasure's
  // mandatory cost supplies the same-turn Apex Dragon payload at no route loss.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, Ability, Supporter, discard, search, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, dynamic DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2232
  expect(outcome.first_ready_turn == 4,
         "Seed 7950 did not reach strict-JIT readiness on T4.");
  expect(trace_contains(trace, "Mega Dragonite ex (Mysterious Treasure cost)"),
         "Seed 7950 did not spend the held Dragon payload on Treasure.");
  expect(!trace_contains(trace, "Mysterious Treasure (Mysterious Treasure cost)"),
         "Seed 7950 still spent the redundant Treasure instead of the payload.");
  expect(trace_contains(trace, "Searched and revealed Crispin"),
         "Seed 7950 did not preserve the Tapu Lele-GX to Crispin connector.");
}

void test_matchup_flex_control_keeps_earlier_t3_finish() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2232 matchup-flex fixture is unavailable.");

  std::mt19937_64 rng(7950);
  sim::Engine engine(*scenario, deck->recipe, rng);
  expect(engine.run().first_ready_turn == 3,
         "Issue 2232 changed the existing earlier matchup-flex finish.");
}
}  // namespace

int main() {
  try {
    test_seed_7950_strict_reaches_t4_with_payload_cost();
    test_matchup_flex_control_keeps_earlier_t3_finish();
    std::cout << "Issue 2232 Treasure two-Energy Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}