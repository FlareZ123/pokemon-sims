from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_issue_1439_treasure_tapu_crispin_override.inc")
source = source_path.read_text(encoding="utf-8")
helper_anchor = "  void run_secret_box_turn() {\n"
helper = dedent(r'''
  bool complete_issue_1596_turo_vessel_dialga_route() {
    if (scenario_.dci != DciProfile::MatchupFlexJit || state_.turn != 3 ||
        item_locked() || !supporter_allowed() || state_.manual_energy_used ||
        !deck_seen_ || !state_.active || state_.active->card != Card::DialgaGX ||
        hand_count(Card::ProfessorTuro) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        need_regi() || need_vstar() || !need_active_vstar() || !need_payload()) {
      return false;
    }
    auto promoted = std::find_if(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar &&
                 pokemon.grass >= 2 && pokemon.fire == 0;
        });
    if (promoted == state_.bench.end()) return false;

    // Professor Turo must resolve before Earthen Vessel. Returning the Active
    // Dialga-GX both promotes the established Regidrago VSTAR and converts Dialga
    // into a route-proven current-turn payload cost for Vessel. Vessel then searches
    // Fire for the unused manual attachment:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, return, promotion, Item, discard, search, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1596
    const Pokemon returned = *state_.active;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;
    for (int energy = 0; energy < returned.grass; ++energy) {
      discard_turo_attachment(Card::Grass);
    }
    for (int energy = 0; energy < returned.fire; ++energy) {
      discard_turo_attachment(Card::Fire);
    }
    if (returned.tool == Tool::ForestSealStone) {
      discard_turo_attachment(Card::ForestSealStone);
    }
    if (returned.tool == Tool::Powerglass) {
      discard_turo_attachment(Card::Powerglass);
    }
    state_.hand.push_back(returned.card);
    state_.active = *promoted;
    state_.bench.erase(promoted);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; P-COMPRESS-01",
          "Professor Turo returned Active Dialga-GX and promoted Regidrago VSTAR before Earthen Vessel.");
    if (!play_earthen_vessel(true)) {
      throw std::logic_error("Issue-1596 Earthen Vessel route disappeared");
    }
    attach_manual();
    return active_is_vstar() && !need_energy() && !need_payload();
  }

''')
if source.count(helper_anchor) != 1:
    raise SystemExit(f"issue-1596 helper anchor count: {source.count(helper_anchor)}")
source = source.replace(helper_anchor, helper + helper_anchor, 1)
run_anchor = "  void run_secret_box_turn() {\n    if (!issue_1439_treasure_tapu_crispin_available()) {\n"
run_insert = "  void run_secret_box_turn() {\n    if (complete_issue_1596_turo_vessel_dialga_route()) {\n      trace(\"POLICY\", \"P-AXIS-01; R-SECRET-BOX-01\",\n            \"End Pineco policy: \" + state_line());\n      return;\n    }\n    if (!issue_1439_treasure_tapu_crispin_available()) {\n"
if source.count(run_anchor) != 1:
    raise SystemExit(f"issue-1596 run anchor count: {source.count(run_anchor)}")
source_path.write_text(source.replace(run_anchor, run_insert, 1), encoding="utf-8")

Path("tests/issue_1596_turo_vessel_dialga_tests.cpp").write_text(dedent(r'''
#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
namespace sim { struct EngineTestAccess {}; }
namespace {
void expect(const bool c, const char* m) { if (!c) throw std::runtime_error(m); }
bool trace_contains(const sim::TraceLog& t, const std::string& x) { return std::any_of(t.lines.begin(), t.lines.end(), [&x](const std::string& l){ return l.find(x) != std::string::npos; }); }
struct SeedResult { sim::TrialOutcome outcome; sim::TraceLog trace; };
SeedResult run_seed(const std::string& deck_id, const std::string& scenario_label, const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label); const sim::NamedDeck* deck = sim::deck_by_id(deck_id);
  expect(scenario.has_value() && deck != nullptr, "The issue-1596 fixture is unavailable.");
  std::mt19937_64 rng(seed); sim::TraceLog trace{true, {}}; sim::Engine engine(*scenario, deck->recipe, rng, &trace); return {engine.run(), std::move(trace)};
}
void test_seed_26_plays_turo_before_vessel() {
  const SeedResult result = run_seed("regidrago-pineco", "matchup-flex-jit/go-second", 26);
  // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1596
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed, "Pineco seed 26 did not reach readiness on turn three.");
  expect(trace_contains(result.trace, "T3 | PLAY SUPPORTER") && trace_contains(result.trace, "Professor Turo returned Active Dialga-GX") && trace_contains(result.trace, "Dialga-GX (Earthen Vessel cost)") && trace_contains(result.trace, "T3 | READY"), "Seed 26 did not preserve the Turo-before-Vessel route.");
}
void test_pineco_seed_35_keeps_existing_t2() { const SeedResult r = run_seed("regidrago-pineco", "strict-jit/go-second", 35); expect(r.outcome.first_ready_turn == 2 && !r.outcome.setup_failed, "Pineco seed 35 lost its existing T2 route."); }
void test_shell_seed_43_keeps_existing_t2() { const SeedResult r = run_seed("regidrago-shell", "strict-jit/go-first", 43); expect(r.outcome.first_ready_turn == 2 && !r.outcome.setup_failed, "Shell seed 43 lost its existing T2 route."); }
}
int main() { test_seed_26_plays_turo_before_vessel(); test_pineco_seed_35_keeps_existing_t2(); test_shell_seed_43_keeps_existing_t2(); return 0; }
'''), encoding="utf-8")
