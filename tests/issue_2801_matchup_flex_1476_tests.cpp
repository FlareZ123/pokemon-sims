#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool t2_route_available(const Engine& engine) {
    return engine.issue_1476_t2_oricorio_first_route_available_after_search_started();
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

sim::State inspected_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::RegidragoVstar,
                sim::Card::QuickBall,
                sim::Card::BrilliantBlender,
                sim::Card::LatiasEx};
  state.discard = {sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Oricorio,
                sim::Card::TapuLeleGX,
                sim::Card::Crispin,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::Dragapult,
                sim::Card::MegaDragonite,
                sim::Card::GoodraVstar};
  state.prizes = {sim::Card::EarthenVessel,
                  sim::Card::ForestSealStone,
                  sim::Card::Lusamine,
                  sim::Card::Arven,
                  sim::Card::Gladion,
                  sim::Card::RegidragoV};
  return state;
}

bool route_available_for(const sim::DciProfile profile,
                         const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-2801", profile, sim::LockMode::None, true, 2};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, inspected_route_state(), true, false);
  return sim::EngineTestAccess::t2_route_available(engine);
}

void test_current_turn_jit_profiles_share_the_route() {
  // StrictJit and MatchupFlexJit both require the Dragon payload to enter the
  // discard pile on the same turn readiness is created. The completed #1476
  // route satisfies that shared contract through Brilliant Blender on T2.
  // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2801
  expect(route_available_for(sim::DciProfile::StrictJit, 280101),
         "The established StrictJit #1476 route was rejected.");
  expect(route_available_for(sim::DciProfile::MatchupFlexJit, 280102),
         "MatchupFlexJit did not receive the shared current-turn JIT route.");
  expect(!route_available_for(sim::DciProfile::NoDiscardControl, 280103),
         "NoDiscardControl incorrectly entered the JIT-only continuation.");
}

void test_matchup_flex_seed_129_reaches_t2() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered MatchupFlexJit seed fixture is unavailable.");

  std::mt19937_64 rng(129);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Treasure spends route-redundant Burnet, establishes K1, then Oricorio,
  // Quick Ball, Tapu Lele-GX, and Crispin complete the two-turn Energy line.
  // Blender supplies the required same-turn Dragon payload on T2.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Core Item, Bench, Ability, attachment, Supporter, evolution, and search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Earliest-route and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2801
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "MatchupFlexJit seed 129 did not reach the legal T2 ready state.");
  expect(trace_contains(trace, "Professor Burnet (Mysterious Treasure cost)"),
         "MatchupFlexJit seed 129 did not spend redundant Burnet.");
  expect(trace_contains(trace, "T1 | WONDER TAG"),
         "MatchupFlexJit seed 129 did not bank Crispin on T1.");
  expect(trace_contains(trace, "T2 | READY"),
         "MatchupFlexJit seed 129 did not reach readiness on T2.");
}

}  // namespace

int main() {
  test_current_turn_jit_profiles_share_the_route();
  test_matchup_flex_seed_129_reaches_t2();
  return 0;
}
