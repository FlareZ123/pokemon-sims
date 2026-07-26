from pathlib import Path
from textwrap import dedent

source_path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
source = source_path.read_text(encoding="utf-8")
helper_anchor = "  bool complete_late_steven_vstar_vessel_continuation() {\n"
helper = dedent(
    '''\
      bool complete_issue_1533_blender_treasure_latias_route() {
        const Pokemon* target = target_regi();
        if (!strict_payload_timing() || item_locked() || !deck_seen_ ||
            target == nullptr || target->card != Card::RegidragoVstar ||
            target->grass < 2 || target->fire < 1 ||
            !need_active_vstar() || !need_payload() || state_.retreat_used ||
            !state_.active || state_.active->card != Card::TapuLeleGX ||
            bench_space() == 0 ||
            !ability_available_for_pokemon(Card::LatiasEx) ||
            in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
            deck_count_after_search_started(Card::LatiasEx) == 0 ||
            hand_count(Card::MysteriousTreasure) == 0 ||
            hand_count(Card::BrilliantBlender) == 0 ||
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
            std::none_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
          return false;
        }

        auto cost = choose_discard(false, true, true, Card::MysteriousTreasure);
        if (!cost) cost = quick_ball_final_surplus_energy_cost();
        if (!cost) return false;

        // The selected Benched Regidrago VSTAR already pays GGF. Mysterious Treasure
        // may spend the route-proven surplus Energy, search Latias ex, and Bench it.
        // Brilliant Blender then establishes the strict-JIT payload, after which
        // Skyliner promotes the completed attacker on the same turn:
        // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
        // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
        // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
        // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Core Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Dynamic DCI and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
        remove_one(state_.hand, Card::MysteriousTreasure);
        state_.discard.push_back(Card::MysteriousTreasure);
        if (!discard_from_hand(*cost, "Mysterious Treasure issue-1533 Latias route cost",
                               "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
          throw std::logic_error("Issue-1533 Treasure cost disappeared");
        }
        record_deck_search_knowledge("Mysterious Treasure issue-1533 Latias route");
        if (!move_deck_to_hand(Card::LatiasEx)) {
          throw std::logic_error("Issue-1533 Latias ex target disappeared");
        }
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
              "Spent the route-proven surplus cost and searched Latias ex.");
        if (!bench_from_hand(Card::LatiasEx, false)) {
          throw std::logic_error("Issue-1533 Latias ex Bench play disappeared");
        }
        if (!play_brilliant_blender()) {
          throw std::logic_error("Issue-1533 Brilliant Blender payload disappeared");
        }
        return retreat_to_benched_vstar_with_latias();
      }

    '''
)
if source.count(helper_anchor) != 1:
    raise SystemExit(f"issue-1533 helper anchor count: {source.count(helper_anchor)}")
source = source.replace(helper_anchor, helper + helper_anchor, 1)
run_anchor = (
    "    play_chaotic_swell();\n"
    "    play_field_blower();\n"
    "    // Resolve the proven K1 Steven route before generic Items can consume Quick Ball\n"
)
run_insert = (
    "    play_chaotic_swell();\n"
    "    play_field_blower();\n"
    "    if (complete_issue_1533_blender_treasure_latias_route()) {\n"
    "      trace(\"POLICY\", \"P-AXIS-01\", \"End: \" + state_line());\n"
    "      return;\n"
    "    }\n"
    "    // Resolve the proven K1 Steven route before generic Items can consume Quick Ball\n"
)
if source.count(run_anchor) != 1:
    raise SystemExit(f"issue-1533 run-turn anchor count: {source.count(run_anchor)}")
source_path.write_text(source.replace(run_anchor, run_insert, 1), encoding="utf-8")

test_source = dedent(
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

    void test_seed_1_uses_treasure_latias_before_blender() {
      const auto scenario = sim::scenario_by_label("strict-jit/go-second");
      const sim::CrobatModelingDeck* deck =
          sim::crobat_modeling_deck_by_id("crobat1-erika");
      expect(scenario.has_value() && deck != nullptr,
             "The issue-1533 fixture is unavailable.");

      std::mt19937_64 rng{1};
      sim::TraceLog trace{true, {}};
      sim::Engine engine(*scenario, deck->recipe, rng, &trace);
      const sim::TrialOutcome outcome = engine.run();

      // Mysterious Treasure may spend the surplus Fire, search Latias ex, and Bench
      // it before Brilliant Blender supplies the current-turn payload. Skyliner then
      // promotes the already-GGF VSTAR on the earliest complete turn:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1533
      expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
             "Seed 1 did not reach strict-JIT readiness on turn four.");
      expect(trace_contains(trace, "Fire Energy (Mysterious Treasure issue-1533 Latias route cost)") &&
                 trace_contains(trace, "T4 | PLAY ITEM | rules: R-MT-01") &&
                 trace_contains(trace, "T4 | BENCH | rules: R-GAME-BENCH | Latias ex") &&
                 trace_contains(trace, "T4 | PLAY ITEM | rules: R-BLENDER-01") &&
                 trace_contains(trace, "T4 | RETREAT | rules: R-LATIAS-01") &&
                 trace_contains(trace, "T4 | READY"),
             "Seed 1 did not execute the Treasure-Latias-Blender completion route.");
    }
    }

    int main() {
      try {
        test_seed_1_uses_treasure_latias_before_blender();
        std::cout << "Issue 1533 Treasure-Latias-Blender tests passed\n";
        return 0;
      } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
      }
    }
    '''
).lstrip()
Path("tests/issue_1533_blender_treasure_latias_tests.cpp").write_text(
    test_source, encoding="utf-8"
)
