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
  static bool issue_2205_available(Engine& engine) {
    return engine.issue_2205_serena_payload_latias_draw_route_available();
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

void exact_seed() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2205 exact-seed fixture unavailable");

  std::mt19937_64 rng{1134};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI and current-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2205
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2205 seed did not reach readiness on T4");
  expect(has(trace, "Serena issue-2205 optional payload discard") &&
             has(trace, "Dragapult ex") &&
             has(trace, "Mysterious Treasure") && has(trace, "Latias ex") &&
             has(trace, "T4 | RETREAT") && has(trace, "T4 | READY"),
         "issue-2205 seed omitted the Serena payload draw route");
}

sim::State boundary_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::Serena, sim::Card::GoodraVstar,
                sim::Card::Channeler, sim::Card::ChaoticSwell,
                sim::Card::Powerglass, sim::Card::EarthenVessel,
                sim::Card::Grass};
  state.deck = {sim::Card::LatiasEx, sim::Card::MysteriousTreasure,
                sim::Card::Fire, sim::Card::RegidragoV};
  state.prizes = {sim::Card::Dipplin};
  return state;
}

void hand_size_boundary() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2205 boundary fixture unavailable");

  std::mt19937_64 rng{2205};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, boundary_state());
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // With H=7, removing Serena plus the ordinary cost leaves five cards; the
  // optional Dragon discard lowers that to four, so Serena gains one draw. With
  // H=8, the same optional discard leaves five and adds no draw at all.
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Blocking review boundary: https://github.com/FlareZ123/pokemon-sims/pull/2218#pullrequestreview-4878965621
  expect(sim::EngineTestAccess::issue_2205_available(engine),
         "issue-2205 rejected the H=7 incremental-draw boundary");

  sim::State eight = boundary_state();
  eight.hand.push_back(sim::Card::QuickBall);
  sim::EngineTestAccess::set_state(engine, std::move(eight));
  expect(!sim::EngineTestAccess::issue_2205_available(engine),
         "issue-2205 admitted H=8 where the Dragon adds zero draws");
}

}  // namespace

int main() {
  exact_seed();
  hand_size_boundary();
  return 0;
}
