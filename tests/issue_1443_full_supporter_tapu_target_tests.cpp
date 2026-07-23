#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};
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

void test_full_supporter_lock_seed_34_preserves_treasure() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-supporter-lock/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1443 fixture is unavailable.");

  std::mt19937_64 rng(34);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Under the repository's full-horizon Supporter lock, Wonder Tag has no
  // playable target. Preserving Mysterious Treasure allows Dragapult ex to pay
  // its printed discard cost on T3 after the third manual Energy attachment:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Full-Supporter-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
  // DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1443
  expect(outcome.first_ready_turn == 3,
         "Full-Supporter-lock seed 34 did not recover its legal T3 route.");
  expect(!trace_contains(trace, "T1 | WONDER TAG"),
         "Full Supporter lock still spent Wonder Tag on an unusable target.");
  expect(!trace_contains(trace,
                         "T1 | PLAY ITEM | rules: R-MT-01; R-GAME-ITEM"),
         "Full Supporter lock still spent the protected Mysterious Treasure on T1.");
  expect(trace_contains(trace,
                        "T3 | DISCARD | rules: R-MT-01 | Dragapult ex "
                        "(Mysterious Treasure cost)"),
         "The preserved Treasure did not establish the strict-JIT payload on T3.");
  expect(trace_contains(trace, "T3 | READY"),
         "The source-bound trace did not report T3 readiness.");
}

void test_unlocked_seed_52_keeps_live_wonder_tag_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The unlocked Wonder Tag control fixture is unavailable.");

  std::mt19937_64 rng(52);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The full-lock guard must leave a live Treasure, Tapu Lele-GX, and Crispin
  // connector untouched in ordinary play:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/issues/1439
  expect(outcome.first_ready_turn == 2,
         "The unlocked seed-52 route lost its earliest ready turn.");
  expect(trace_contains(trace, "T2 | WONDER TAG"),
         "The full-lock target guard suppressed an unlocked live Wonder Tag route.");
}

}  // namespace

int main() {
  try {
    test_full_supporter_lock_seed_34_preserves_treasure();
    test_unlocked_seed_52_keeps_live_wonder_tag_route();
    std::cout << "Issue 1443 full-Supporter-lock target tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
