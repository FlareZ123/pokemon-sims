from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
source = source_path.read_text(encoding="utf-8")
helper_anchor = "  bool complete_late_steven_vstar_vessel_continuation() {\n"
helper = dedent(
    '''\
      bool complete_issue_1564_quick_ball_latias_promotion() {
        if (scenario_.dci != DciProfile::NoDiscardControl || item_locked() ||
            !deck_seen_ || hand_count(Card::QuickBall) == 0 ||
            hand_count(Card::Grass) == 0 || !need_active_vstar() ||
            need_energy() || need_payload() || state_.retreat_used ||
            !state_.active || state_.active->card != Card::TapuLeleGX ||
            bench_space() == 0 ||
            !ability_available_for_pokemon(Card::LatiasEx) ||
            in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
            deck_count_after_search_started(Card::LatiasEx) == 0) {
          return false;
        }
        Pokemon* target = best_benched_vstar_for_promotion();
        if (target == nullptr || target->grass < 2 || target->fire < 1) {
          return false;
        }

        // The payload is already banked under no-discard-control and the Benched
        // Regidrago VSTAR already pays GGF. The remaining Grass is therefore a
        // route-proven high-DCI Quick Ball cost. Search and Bench Latias ex, then
        // Skyliner removes the Basic Active Tapu Lele-GX's Retreat Cost so the ready
        // VSTAR can be promoted during the same turn:
        // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
        // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
        // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
        // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Item, discard, search, Bench, Ability, retreat, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // No-control payload banking, DCI, and shortest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1564
        remove_one(state_.hand, Card::QuickBall);
        state_.discard.push_back(Card::QuickBall);
        if (!discard_from_hand(Card::Grass,
                               "Quick Ball issue-1564 Latias route cost",
                               "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
          throw std::logic_error("Issue-1564 surplus Grass cost disappeared");
        }
        record_deck_search_knowledge("Quick Ball issue-1564 Latias route");
        if (!move_deck_to_hand(Card::LatiasEx)) {
          throw std::logic_error("Issue-1564 Latias ex target disappeared");
        }
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01",
              "Spent surplus Grass Energy and searched Latias ex for the immediate promotion.");
        if (!bench_from_hand(Card::LatiasEx, false)) {
          throw std::logic_error("Issue-1564 Latias ex Bench action failed");
        }
        if (!retreat_to_benched_vstar_with_latias()) {
          throw std::logic_error("Issue-1564 Skyliner promotion disappeared");
        }
        return active_is_vstar() && !need_energy() && !need_payload();
      }

    '''
)
if source.count(helper_anchor) != 1:
    raise SystemExit(f"issue-1564 helper anchor count: {source.count(helper_anchor)}")
source = source.replace(helper_anchor, helper + helper_anchor, 1)
run_anchor = (
    "    attach_manual();\n"
    "    // A manual attachment can complete GGF and thereby make a held Mysterious\n"
)
run_insert = (
    "    attach_manual();\n"
    "    if (complete_issue_1564_quick_ball_latias_promotion()) {\n"
    "      trace(\"POLICY\", \"P-AXIS-01\", \"End: \" + state_line());\n"
    "      return;\n"
    "    }\n"
    "    // A manual attachment can complete GGF and thereby make a held Mysterious\n"
)
if source.count(run_anchor) != 1:
    raise SystemExit(f"issue-1564 run anchor count: {source.count(run_anchor)}")
source_path.write_text(source.replace(run_anchor, run_insert, 1), encoding="utf-8")

Path("tests/issue_1564_quick_ball_latias_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"

        #include <algorithm>
        #include <random>
        #include <stdexcept>
        #include <string>

        namespace sim { struct EngineTestAccess {}; }

        namespace {
        void expect(bool condition, const char* message) {
          if (!condition) throw std::runtime_error(message);
        }
        bool has(const sim::TraceLog& trace, const std::string& needle) {
          return std::any_of(trace.lines.begin(), trace.lines.end(),
                             [&](const std::string& line) {
                               return line.find(needle) != std::string::npos;
                             });
        }
        void exact_seed() {
          const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
          const auto* deck = sim::deck_by_id("regidrago-shell");
          expect(scenario && deck, "issue-1564 fixture unavailable");
          std::mt19937_64 rng{5};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          const auto outcome = engine.run();
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
          // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
          // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1564
          expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
                 "seed 5 did not reach no-control readiness on T3");
          expect(has(trace, "T3 | DISCARD") &&
                     has(trace, "Quick Ball issue-1564 Latias route cost") &&
                     has(trace, "T3 | BENCH") && has(trace, "Latias ex") &&
                     has(trace, "T3 | RETREAT") && has(trace, "T3 | READY"),
                 "seed 5 omitted the Quick Ball-Latias promotion");
        }
        }
        int main() { exact_seed(); }
        '''
    ).lstrip(),
    encoding="utf-8",
)
