#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim { struct EngineTestAccess {}; }

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void exact_seed(const char* scenario_label) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2164 fixture unavailable");

  std::mt19937_64 rng{69};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, route priority, and DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2164
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "seed 69 did not reach readiness on T4");
  expect(has(trace, "Quick Ball issue-2164 current-turn payload cost") &&
             has(trace, "T4 | QUICK BALL") &&
             has(trace, "searched Latias ex") &&
             has(trace, "T4 | BENCH") && has(trace, "Latias ex") &&
             has(trace, "T4 | RETREAT") && has(trace, "T4 | READY"),
         "seed 69 omitted the deterministic Quick Ball-Latias finish");
  expect(!has(trace, "T4 | WONDER TAG") &&
             !has(trace, "T4 | PLAY SUPPORTER | Serena"),
         "seed 69 still consumed the weaker Tapu-Supporter route");
}
}  // namespace

int main() {
  exact_seed("strict-jit/go-first");
  exact_seed("matchup-flex-jit/go-first");
}
