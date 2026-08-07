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

void test_strict_seed_3963_reaches_earliest_t3() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2227 strict fixture is unavailable.");

  std::mt19937_64 rng(3963);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T1 Quick Ball has already established K1. On T3, Treasure can therefore
  // legally know the full Tapu Lele-GX -> Wonder Tag -> Crispin continuation.
  // Spending Dragapult ex as Treasure's mandatory cost supplies the current-turn
  // payload while Crispin attaches the missing Fire to the Active GG VSTAR. The
  // exact source-bound replay proves T3 is the earliest completed route for this
  // fixed seed, so the regression follows the repository's earliest-readiness goal.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Ability, Supporter, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Non-retroactive K1, dynamic DCI/JIT, and earliest-readiness policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2227
  expect(outcome.first_ready_turn == 3,
         "Strict seed 3963 did not reach its earliest legal readiness on T3.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)"),
         "Strict seed 3963 did not spend Dragapult ex as Treasure's cost.");
  expect(trace_contains(trace, "T3 | READY"),
         "Strict seed 3963 did not record readiness on T3.");
  expect(!trace_contains(trace, "Dipplin TWM 127 (Mysterious Treasure cost)"),
         "Strict seed 3963 still spent Dipplin instead of the payload.");
}

void test_matchup_flex_seed_3963_keeps_t4() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  expect(deck != nullptr && scenario.has_value(),
         "The registered issue-2227 matchup-flex fixture is unavailable.");
  std::mt19937_64 rng(3963);
  sim::Engine engine(*scenario, deck->recipe, rng);
  expect(engine.run().first_ready_turn == 4,
         "Issue 2227 regressed the existing matchup-flex T4 finish.");
}
}  // namespace

int main() {
  try {
    test_strict_seed_3963_reaches_earliest_t3();
    test_matchup_flex_seed_3963_keeps_t4();
    std::cout << "Issue 2227 GG Treasure Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}