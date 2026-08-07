#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool issue_2199_available(Engine& engine) {
    return engine.issue_2199_quick_ball_latias_payload_route_available();
  }
};

}  // namespace sim

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

void exact_seed(const char* scenario_label, const std::uint64_t seed,
                const char* payload_name) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2199 exact-seed fixture unavailable");

  std::mt19937_64 rng{seed};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Bench, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, current-turn JIT, DCI, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2199
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "issue-2199 seed did not reach readiness on T3");
  expect(has(trace, "Quick Ball issue-2199 current-turn payload cost") &&
             has(trace, payload_name) && has(trace, "searched Latias ex") &&
             has(trace, "T3 | BENCH") && has(trace, "Latias ex") &&
             has(trace, "T3 | RETREAT") && has(trace, "T3 | READY"),
         "issue-2199 seed omitted the Dragon-cost Latias finish");
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::QuickBall, sim::Card::GoodraVstar,
                sim::Card::Channeler};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass};
  state.prizes = {sim::Card::Dipplin};
  return state;
}

void k1_and_k0_controls() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2199 control fixture unavailable");

  std::mt19937_64 rng{2199};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // The targeted selector uses the exact post-inspection deck and cannot infer a
  // hidden Latias ex at K0:
  // Hisuian Heavy Ball / Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // K0/K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2199
  sim::EngineTestAccess::set_knowledge(engine, false, false);
  expect(!sim::EngineTestAccess::issue_2199_available(engine),
         "issue-2199 route used hidden K0 deck identity");
  sim::EngineTestAccess::set_knowledge(engine, true, false);
  expect(sim::EngineTestAccess::issue_2199_available(engine),
         "issue-2199 route rejected the exact K1 Latias target");
}

}  // namespace

int main() {
  exact_seed("matchup-flex-jit/go-second", 5, "Mega Dragonite ex");
  exact_seed("strict-jit/go-second", 35, "Hisuian Goodra VSTAR");
  k1_and_k0_controls();
  return 0;
}
