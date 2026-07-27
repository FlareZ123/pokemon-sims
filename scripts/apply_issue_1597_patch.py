from pathlib import Path
from textwrap import dedent

supporter_path = Path("src/trace_engine_v2/part_010_steven_crispin_override.inc")
supporter = supporter_path.read_text(encoding="utf-8")
supporter_anchor = (
    "  Card choose_supporter_after_search_started() {\n\n"
    "if (wonder_tag_can_bank_steven_for_known_t3_active_regi_route()) {\n"
)
supporter_insert = dedent(
    '''\
      Card choose_supporter_after_search_started() {

        const bool issue_1597_banked_steven_turo_route =
            scenario_.dci == DciProfile::NoDiscardControl &&
            scenario_.going_first && state_.turn == 1 &&
            scenario_.locks == LockMode::None && scenario_.max_turn >= 4 &&
            !supporter_allowed() && deck_seen_ && state_.active &&
            state_.active->card == Card::Oricorio &&
            in_play_count(Card::RegidragoV) > 0 &&
            hand_count(Card::ProfessorTuro) > 0 &&
            hand_count(Card::EarthenVessel) > 0 && !item_locked() &&
            deck_count_after_search_started(Card::Grass) > 0 &&
            deck_count_after_search_started(Card::Fire) > 0 &&
            need_vstar() && need_energy() &&
            std::any_of(state_.discard.begin(), state_.discard.end(), is_payload) &&
            deck_count_after_search_started(Card::StevensResolve) > 0 &&
            deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
            deck_count_after_search_started(Card::Crispin) > 0;
        if (issue_1597_banked_steven_turo_route) {
          // Going first prevents the T1 Supporter play, so Wonder Tag banks Steven's
          // Resolve for T2. Steven searches Regidrago VSTAR and Crispin, Crispin supplies
          // the final Grass on T3, and the already-held Professor Turo promotes the
          // complete attacker on T4 without same-turn Supporter contention:
          // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
          // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
          // Core first-turn Supporter, search, attachment, evolution, return, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Supporter schedule, K1, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1597
          return Card::StevensResolve;
        }

    if (wonder_tag_can_bank_steven_for_known_t3_active_regi_route()) {
    '''
)
if supporter.count(supporter_anchor) != 1:
    raise SystemExit(f"issue-1597 supporter anchor count: {supporter.count(supporter_anchor)}")
supporter_path.write_text(
    supporter.replace(supporter_anchor, supporter_insert, 1), encoding="utf-8"
)

turn_path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
turn_source = turn_path.read_text(encoding="utf-8")
turn_anchor = dedent(
    '''\
      void run_turn() {
        if (secret_box_combo_enabled()) {
          run_secret_box_turn();
          return;
        }
        trace("POLICY", "P-AXIS-01", "Start: " + state_line());
        if (complete_late_steven_vstar_vessel_continuation()) return;
    '''
)
turn_insert = dedent(
    '''\
      bool complete_issue_1597_banked_steven_turn() {
        if (scenario_.dci != DciProfile::NoDiscardControl ||
            !scenario_.going_first || scenario_.locks != LockMode::None ||
            state_.turn != 2 || !supporter_allowed() || state_.manual_energy_used ||
            !deck_seen_ || !state_.active || state_.active->card != Card::Oricorio ||
            !std::any_of(state_.discard.begin(), state_.discard.end(), is_payload) ||
            hand_count(Card::StevensResolve) == 0 || hand_count(Card::Fire) == 0 ||
            hand_count(Card::ProfessorTuro) == 0 || !need_vstar() || !need_energy() ||
            deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
            deck_count_after_search_started(Card::Crispin) == 0) {
          return false;
        }

        // The T1 Wonder Tag banked Steven because the first player could not play a
        // Supporter. Attach the held Fire, then Steven searches the missing VSTAR and
        // the next-turn Crispin while preserving the later Active-position connector:
        // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Supporter, search, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Supporter schedule and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1597
        attach_manual();
        remove_one(state_.hand, Card::StevensResolve);
        state_.discard.push_back(Card::StevensResolve);
        state_.supporter_used = true;
        record_deck_search_knowledge("Steven's Resolve issue-1597 schedule");
        if (!move_deck_to_hand(Card::RegidragoVstar) ||
            !move_deck_to_hand(Card::Crispin)) {
          throw std::logic_error("Issue-1597 Steven targets disappeared");
        }
        shuffle(state_.deck);
        state_.turn_ended = true;
        trace("PLAY SUPPORTER", "R-STEVEN-01; P-COMPRESS-01",
              "Steven's Resolve banked Regidrago VSTAR and Crispin for the next Energy turn; the turn ended.");
        return true;
      }

      void run_turn() {
        if (secret_box_combo_enabled()) {
          run_secret_box_turn();
          return;
        }
        trace("POLICY", "P-AXIS-01", "Start: " + state_line());
        if (complete_issue_1597_banked_steven_turn()) {
          trace("TURN END", "R-STEVEN-01", state_line());
          return;
        }
        if (complete_late_steven_vstar_vessel_continuation()) return;
    '''
)
if turn_source.count(turn_anchor) != 1:
    raise SystemExit(f"issue-1597 turn anchor count: {turn_source.count(turn_anchor)}")
turn_path.write_text(turn_source.replace(turn_anchor, turn_insert, 1), encoding="utf-8")

Path("tests/issue_1597_wonder_tag_steven_turo_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"
        #include <algorithm>
        #include <random>
        #include <stdexcept>
        #include <string>
        #include <utility>
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
        struct Result { sim::TrialOutcome outcome; sim::TraceLog trace; };
        Result run(const std::string& variant, const std::string& scenario,
                   std::uint64_t seed) {
          auto selected_scenario = sim::scenario_by_label(scenario);
          const auto* deck = sim::crobat_modeling_deck_by_id(variant);
          expect(selected_scenario && deck, "fixture");
          std::mt19937_64 rng(seed);
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*selected_scenario, deck->recipe, rng, &trace);
          return {engine.run(), std::move(trace)};
        }
        void exact() {
          auto result = run("crobat2-erika-channeler",
                            "no-discard-control/go-first", 27);
          // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
          // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
          // Professor Turo: https://api.pokemontcg.io/v2/cards/sv4-171
          // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1597
          expect(result.outcome.first_ready_turn > 0 &&
                     result.outcome.first_ready_turn <= 4 &&
                     !result.outcome.setup_failed,
                 "seed27 missed the Steven schedule deadline");
          expect(has(result.trace, "T1 | WONDER TAG") &&
                     has(result.trace, "Searched and revealed Steven's Resolve") &&
                     has(result.trace, "T2 | PLAY SUPPORTER") &&
                     has(result.trace, "banked Regidrago VSTAR and Crispin") &&
                     has(result.trace, "READY"),
                 "route absent");
        }
        void controls() {
          auto second = run("crobat2-erika-channeler",
                            "no-discard-control/go-second", 27);
          expect(!has(second.trace, "banked Regidrago VSTAR and Crispin"),
                 "go-second used the going-first route");
          auto strict = run("crobat2-erika-channeler",
                            "strict-jit/go-first", 27);
          expect(!has(strict.trace, "banked Regidrago VSTAR and Crispin"),
                 "strict used the no-control route");
        }
        }
        int main() { exact(); controls(); }
        '''
    ).lstrip(),
    encoding="utf-8",
)
