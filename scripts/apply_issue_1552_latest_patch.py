from pathlib import Path
from textwrap import dedent

items_path = Path("src/trace_engine_v2/part_014a.inc")
items = items_path.read_text(encoding="utf-8")
items_anchor = (
    "      if (bench_oricorio_if_useful()) return true;\n"
    "      if (play_pokemon_communication(permit_payload)) return true;\n\n"
)
items_insert = dedent(
    '''\
          if (bench_oricorio_if_useful()) return true;
          if (play_pokemon_communication(permit_payload)) return true;

          const auto issue_1552_t1_vessel_route_available = [this] {
            if (secret_box_combo_enabled() ||
                scenario_.dci != DciProfile::StrictJit ||
                scenario_.locks != LockMode::None || !scenario_.going_first ||
                state_.turn != 1 || scenario_.max_turn < 2 || item_locked() ||
                state_.manual_energy_used || !state_.active ||
                state_.active->card != Card::RegidragoV ||
                state_.active->grass != 0 || state_.active->fire != 0 ||
                bench_space() <= 0 ||
                !ability_available_for_pokemon(Card::TapuLeleGX) ||
                hand_count(Card::EarthenVessel) == 0 ||
                hand_count(Card::MysteriousTreasure) == 0 ||
                hand_count(Card::RegidragoVstar) == 0 ||
                hand_count(Card::QuickBall) == 0 ||
                !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
              return false;
            }

            // At K0, use only fixed-list and public-zone availability. Earthen Vessel
            // may spend Mysterious Treasure because held Regidrago VSTAR replaces its
            // evolution role, while Quick Ball preserves the T2 Tapu Lele-GX connector
            // and simultaneously places the held Dragon payload in discard. Vessel's
            // T1 search establishes K1 before the T2 Tapu, Crispin, and Energy choices:
            // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
            // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
            // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
            // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
            // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
            // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
            // Core first-turn, Item, evolution, Supporter, Bench, Ability, search, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
            // K0/K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
            // Dynamic DCI and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
            // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1552
            return might_be_unseen(Card::Grass) &&
                might_be_unseen(Card::Fire) &&
                might_be_unseen(Card::TapuLeleGX) &&
                might_be_unseen(Card::Crispin);
          };

          const auto play_issue_1552_t1_vessel_route =
              [this, &issue_1552_t1_vessel_route_available] {
            if (!issue_1552_t1_vessel_route_available()) return false;
            if (!remove_one(state_.hand, Card::EarthenVessel)) {
              throw std::logic_error(
                  "projected issue-1552 Earthen Vessel disappeared");
            }
            state_.discard.push_back(Card::EarthenVessel);
            if (!discard_from_hand(
                    Card::MysteriousTreasure,
                    "Earthen Vessel issue-1552 route cost",
                    "R-EV-01; P-DCI-01; P-COMPRESS-01")) {
              throw std::logic_error(
                  "projected issue-1552 Mysterious Treasure cost disappeared");
            }
            search_energy_to_hand(
                2, "R-EV-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
                "Earthen Vessel issue-1552 T1 route");
            return true;
          };

          if (play_issue_1552_t1_vessel_route()) return true;
    '''
)
if items.count(items_anchor) != 1:
    raise SystemExit(f"issue-1552 items anchor count: {items.count(items_anchor)}")
items_path.write_text(items.replace(items_anchor, items_insert, 1), encoding="utf-8")

attach_path = Path("src/trace_engine_v2/part_tate_blender_attachment_override.inc")
attach = attach_path.read_text(encoding="utf-8")
attach_anchor = "  bool attach_manual() {\n"
attach_insert = dedent(
    '''\
      bool attach_manual() {
        const bool issue_1552_public_t2_state =
            !secret_box_combo_enabled() &&
            scenario_.dci == DciProfile::StrictJit &&
            scenario_.locks == LockMode::None && scenario_.going_first &&
            state_.turn == 2 && deck_seen_ && prizes_known() &&
            !state_.manual_energy_used && supporter_allowed() &&
            state_.active && state_.active->card == Card::RegidragoV &&
            state_.active->entered_turn < state_.turn &&
            state_.active->grass == 1 && state_.active->fire == 0 &&
            bench_space() > 0 &&
            ability_available_for_pokemon(Card::TapuLeleGX) &&
            count_of(state_.discard, Card::EarthenVessel) > 0 &&
            count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
            hand_count(Card::RegidragoVstar) > 0 &&
            hand_count(Card::QuickBall) > 0 && hand_count(Card::Fire) > 0 &&
            deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
            deck_count_after_search_started(Card::Crispin) > 0 &&
            deck_count_after_search_started(Card::Grass) > 0 &&
            deck_count_after_search_started(Card::Fire) > 0;
        const auto issue_1552_quick_ball_cost = issue_1552_public_t2_state
            ? choose_discard(true, true, true, Card::QuickBall)
            : std::optional<Card>{};
        const bool issue_1552_t2_vessel_finish =
            issue_1552_quick_ball_cost &&
            is_payload(*issue_1552_quick_ball_cost);
        if (issue_1552_t2_vessel_finish) {
          // The T1 Earthen Vessel search established K1 and attached the first Grass.
          // Before spending the T2 manual attachment, evolve, let Quick Ball discard a
          // current-turn Dragon payload, Bench Tapu Lele-GX, resolve Wonder Tag for
          // Crispin, and let Crispin attach the second Grass. The ordinary attachment
          // then supplies the held Fire and completes GGF on the earliest legal turn:
          // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Evolution, Item, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // K1, DCI, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1552
          if (!evolve_best_regi()) {
            throw std::logic_error(
                "projected issue-1552 T2 evolution disappeared");
          }
          if (!remove_one(state_.hand, Card::QuickBall)) {
            throw std::logic_error(
                "projected issue-1552 Quick Ball disappeared");
          }
          state_.discard.push_back(Card::QuickBall);
          if (!discard_from_hand(
                  *issue_1552_quick_ball_cost,
                  "Quick Ball issue-1552 route cost",
                  "R-QB-01; P-DCI-01; P-JIT-01")) {
            throw std::logic_error(
                "projected issue-1552 Dragon payload cost disappeared");
          }
          record_deck_search_knowledge("Quick Ball issue-1552 T2 route");
          if (!move_deck_to_hand(Card::TapuLeleGX)) {
            throw std::logic_error(
                "projected issue-1552 Tapu Lele-GX target disappeared");
          }
          shuffle(state_.deck);
          trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM",
                "Searched a Basic Pokémon: Tapu Lele-GX.");
          if (!bench_from_hand(Card::TapuLeleGX, true)) {
            throw std::logic_error(
                "projected issue-1552 Tapu Lele-GX Bench play disappeared");
          }
          if (hand_count(Card::Crispin) == 0 || !play_crispin()) {
            throw std::logic_error(
                "projected issue-1552 Wonder Tag to Crispin route disappeared");
          }
        }
    '''
)
if attach.count(attach_anchor) != 1:
    raise SystemExit(f"issue-1552 attach anchor count: {attach.count(attach_anchor)}")
attach_path.write_text(attach.replace(attach_anchor, attach_insert, 1), encoding="utf-8")

Path("tests/issue_1552_t1_vessel_route_tests.cpp").write_text(
    dedent(
        r'''
        #define REGIDRAGO_SIM_NO_MAIN
        #include "../src/regidrago_sim.cpp"

        #include <algorithm>
        #include <iostream>
        #include <random>
        #include <stdexcept>
        #include <string>

        namespace sim { struct EngineTestAccess {}; }

        namespace {
        void expect(const bool condition, const char* message) {
          if (!condition) throw std::runtime_error(message);
        }
        bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
          return std::any_of(trace.lines.begin(), trace.lines.end(),
                             [&needle](const std::string& line) {
                               return line.find(needle) != std::string::npos;
                             });
        }
        void test_seed_104_uses_t1_vessel_and_reaches_t2() {
          const auto scenario = sim::scenario_by_label("strict-jit/go-first");
          const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
          expect(scenario.has_value() && deck != nullptr,
                 "The issue-1552 fixture is unavailable.");
          std::mt19937_64 rng{104};
          sim::TraceLog trace{true, {}};
          sim::Engine engine(*scenario, deck->recipe, rng, &trace);
          const sim::TrialOutcome outcome = engine.run();
          // Earthen Vessel can spend route-replaced Mysterious Treasure on T1, establish
          // K1, and preserve Quick Ball for the T2 Tapu Lele-GX to Crispin continuation:
          // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
          // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
          // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
          // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1552
          expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
                 "Seed 104 did not reach strict-JIT readiness on turn two.");
          expect(trace_contains(trace, "Mysterious Treasure (Earthen Vessel issue-1552 route cost)") &&
                     trace_contains(trace, "Quick Ball issue-1552 route cost") &&
                     trace_contains(trace, "T2 | WONDER TAG") &&
                     trace_contains(trace, "T2 | READY |"),
                 "Seed 104 did not execute the source-bound Vessel to Quick Ball route.");
        }
        }
        int main() {
          try {
            test_seed_104_uses_t1_vessel_and_reaches_t2();
            std::cout << "Issue 1552 T1 Vessel route tests passed\n";
            return 0;
          } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
            return 1;
          }
        }
        '''
    ).lstrip(),
    encoding="utf-8",
)
