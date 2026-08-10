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

bool trace_has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_939_stronger_current_route_survives() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered #2224 compatibility fixture is unavailable.");

  std::mt19937_64 rng(939);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool preserved_route =
      outcome.first_ready_turn == 4 && !outcome.setup_failed &&
      trace_has(trace, "T4 | PLAY SUPPORTER | rules: R-GLADION-01") &&
      trace_has(trace, "Mega Dragonite ex (Earthen Vessel cost)") &&
      trace_has(trace, "T4 | READY");
  if (!preserved_route) {
    for (const std::string& line : trace.lines) std::cerr << line << '\n';
  }

  // The #2891 Star Alchemy route may be generalized only while it preserves the
  // earliest complete observable route. Seed 939 already has the proven K1
  // Gladion -> Earthen Vessel route whose Vessel cost discards the held Dragon
  // and whose manual Grass attachment completes strict-JIT GGF on T4:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Advanced Item, Supporter, discard, search, attachment, evolution, and Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1 and earliest-complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Established stronger route: https://github.com/FlareZ123/pokemon-sims/issues/2224
  // #2891 explicitly requires preserving stronger immediate routes: https://github.com/FlareZ123/pokemon-sims/issues/2891
  expect(preserved_route,
         "#2891 preempted the established #2224 T4 held-Vessel finish.");
}

}  // namespace

int main() {
  test_seed_939_stronger_current_route_survives();
}
