from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
source = source_path.read_text(encoding="utf-8")

helper_anchor = "  void run_turn() {\n"
helper = dedent(r'''
  bool complete_issue_1599_quick_ball_tapu_crispin_route() {
    if (scenario_.dci != DciProfile::MatchupFlexJit || state_.turn != 2 ||
        item_locked() || !supporter_allowed() || !deck_seen_ ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass < 2 || state_.active->fire != 0 ||
        need_regi() || need_vstar() || need_active_vstar() || need_payload() ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::GoodraVstar) == 0 ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::TapuLeleGX) ||
        in_play(Card::TapuLeleGX) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    // Reopen the costed Basic-search channel after Mysterious Treasure has
    // established the VSTAR and current-turn payload axes. Goodra is then a
    // route-proven high-DCI cost, Tapu Lele-GX supplies Wonder Tag, and Crispin
    // attaches the sole missing Fire Energy during the unused Supporter action:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, search, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1599
    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::GoodraVstar,
                           "Quick Ball issue-1599 Tapu-Crispin route cost",
                           "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1599 Quick Ball cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1599 Tapu-Crispin route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue-1599 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01",
          "Spent Hisuian Goodra VSTAR and searched Tapu Lele-GX for the immediate Crispin finish.");

    const int crispin_before = hand_count(Card::Crispin);
    if (!bench_from_hand(Card::TapuLeleGX, true) ||
        hand_count(Card::Crispin) <= crispin_before) {
      throw std::logic_error("Issue-1599 Wonder Tag did not obtain Crispin");
    }
    if (!play_crispin()) {
      throw std::logic_error("Issue-1599 Crispin completion disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

''')
if source.count(helper_anchor) != 1:
    raise SystemExit(f"issue-1599 helper anchor count: {source.count(helper_anchor)}")
source = source.replace(helper_anchor, helper + helper_anchor, 1)

run_anchor = (
    "    evolve_best_regi();\n"
    "    if (late_steven_active_vstar_crispin_treasure_route_available() &&\n"
)
run_insert = (
    "    evolve_best_regi();\n"
    "    if (complete_issue_1599_quick_ball_tapu_crispin_route()) {\n"
    "      trace(\"POLICY\", \"P-AXIS-01\", \"End: \" + state_line());\n"
    "      return;\n"
    "    }\n"
    "    if (late_steven_active_vstar_crispin_treasure_route_available() &&\n"
)
if source.count(run_anchor) != 1:
    raise SystemExit(f"issue-1599 run anchor count: {source.count(run_anchor)}")
source_path.write_text(source.replace(run_anchor, run_insert, 1), encoding="utf-8")

Path("tests/issue_1599_quick_ball_tapu_crispin_tests.cpp").write_text(
    dedent(r'''
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

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
struct SeedResult { sim::TrialOutcome outcome; sim::TraceLog trace; };
SeedResult run_seed(const std::string& scenario_label, const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-1599 fixture is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}
void test_seed_24_finishes_on_turn_two() {
  const SeedResult result = run_seed("matchup-flex-jit/go-second", 24);
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1599
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Seed 24 did not reach matchup-flex readiness on turn two.");
  expect(trace_contains(result.trace,
                        "Goodra VSTAR (Quick Ball issue-1599 Tapu-Crispin route cost)") &&
             trace_contains(result.trace, "T2 | WONDER TAG") &&
             trace_contains(result.trace, "T2 | PLAY SUPPORTER") &&
             trace_contains(result.trace, "T2 | READY"),
         "Seed 24 did not use the complete Quick Ball-Tapu-Crispin route.");
}
void test_strict_seed_43_preserves_existing_turn_two_route() {
  const SeedResult result = run_seed("strict-jit/go-first", 43);
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Strict seed 43 lost its existing turn-two route.");
}
void test_no_control_seed_42_preserves_quick_ball_hold() {
  const SeedResult result = run_seed("no-discard-control/go-first", 42);
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "No-control seed 42 lost its resource-preserving turn-three route.");
  expect(trace_contains(result.trace, "HOLD QUICK BALL"),
         "No-control seed 42 no longer preserves its redundant Quick Ball.");
}
}  // namespace
int main() {
  test_seed_24_finishes_on_turn_two();
  test_strict_seed_43_preserves_existing_turn_two_route();
  test_no_control_seed_42_preserves_quick_ball_hold();
  return 0;
}
'''), encoding="utf-8")
