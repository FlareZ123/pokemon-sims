from pathlib import Path
from textwrap import dedent

selector_path = Path("src/trace_engine_v2/part_012.inc")
selector = selector_path.read_text(encoding="utf-8")
member_anchor = "  bool play_gladion() {\n"
member_insert = dedent(
    '''\
      bool issue_1598_bank_prized_treasure_{false};

      bool play_gladion() {
    '''
)
if selector.count(member_anchor) != 1:
    raise SystemExit(
        f"issue-1598 member anchor count: {selector.count(member_anchor)}"
    )
selector = selector.replace(member_anchor, member_insert, 1)

selector_anchor = (
    "    if (!known_target && revealed_missing_axis_target) known_target = revealed_missing_axis_target;\n\n"
    "    // Gladion reveals every Prize before selecting the exchange, so a newly revealed\n"
)
selector_insert = dedent(
    '''\
        if (!known_target && revealed_missing_axis_target) known_target = revealed_missing_axis_target;

        const bool revealed_treasure_next_turn_completion =
            !known_target && !revealed_missing_axis_target && strict_payload_timing() &&
            !item_locked() && state_.turn < scenario_.max_turn && state_.active &&
            state_.active->card == Card::RegidragoV &&
            state_.active->entered_turn < state_.turn &&
            state_.active->grass >= 2 && state_.active->fire == 0 &&
            need_vstar() && need_energy() && need_payload() &&
            hand_count(Card::Fire) > 0 &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
            prize_count_after_reveal(Card::MysteriousTreasure) > 0 &&
            deck_count_after_search_started(Card::RegidragoVstar) > 0;
        if (revealed_treasure_next_turn_completion) {
          // Gladion has revealed a deterministic next-turn route without consulting
          // Celestial Roar's hidden top cards. The held Fire completes GGF next turn,
          // while the recovered Mysterious Treasure discards the held Dragon payload,
          // searches Regidrago VSTAR, and permits evolution of the prior-turn Active:
          // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-166
          // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Core Prize exchange, Item cost, Energy attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
          // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1598
          known_target = Card::MysteriousTreasure;
        }

        // Gladion reveals every Prize before selecting the exchange, so a newly revealed
    '''
)
if selector.count(selector_anchor) != 1:
    raise SystemExit(
        f"issue-1598 selector anchor count: {selector.count(selector_anchor)}"
    )
selector = selector.replace(selector_anchor, selector_insert, 1)

exchange_anchor = "    if (known_target && exchange(*known_target)) return true;\n"
exchange_insert = dedent(
    '''\
        if (known_target && exchange(*known_target)) {
          // Record only the exact Gladion recovery that creates this deferred route.
          // This keeps unrelated Steven and Treasure states on their established policy:
          // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1598
          issue_1598_bank_prized_treasure_ =
              *known_target == Card::MysteriousTreasure;
          return true;
        }
    '''
)
if selector.count(exchange_anchor) != 1:
    raise SystemExit(
        f"issue-1598 exchange anchor count: {selector.count(exchange_anchor)}"
    )
selector_path.write_text(
    selector.replace(exchange_anchor, exchange_insert, 1), encoding="utf-8"
)

item_path = Path("src/trace_engine_v2/part_009a.inc")
item_source = item_path.read_text(encoding="utf-8")
item_anchor = (
    "  bool play_mysterious_treasure(const bool permit_payload) {\n"
    "    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;\n"
)
item_insert = dedent(
    '''\
      bool play_mysterious_treasure(const bool permit_payload) {
        if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
        const bool bank_for_deterministic_next_turn_payload =
            issue_1598_bank_prized_treasure_ && strict_payload_timing() &&
            state_.turn < scenario_.max_turn && state_.manual_energy_used &&
            state_.active && state_.active->card == Card::RegidragoV &&
            state_.active->entered_turn < state_.turn &&
            state_.active->grass >= 2 && state_.active->fire == 0 &&
            need_vstar() && need_payload() && hand_count(Card::Fire) > 0 &&
            std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
            might_be_unseen(Card::RegidragoVstar);
        if (bank_for_deterministic_next_turn_payload) {
          // Preserve the Treasure recovered by Gladion for the exact public route.
          // Playing it now would discard the held Dragon one turn before strict-JIT
          // permits that cost and would make T3 depend on an unknown Legacy Star mill.
          // On the next turn, attach Fire, discard the held Dragon to Treasure, search
          // VSTAR, evolve the prior-turn Active, and reach deterministic readiness:
          // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
          // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
          // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-166
          // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
          // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Core Item, discard, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
          // Dynamic DCI and strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
          // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1598
          return false;
        }
    '''
)
if item_source.count(item_anchor) != 1:
    raise SystemExit(
        f"issue-1598 Treasure anchor count: {item_source.count(item_anchor)}"
    )
item_source = item_source.replace(item_anchor, item_insert, 1)
clear_anchor = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\n"
)
clear_insert = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\n"
    "    issue_1598_bank_prized_treasure_ = false;\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\n"
)
if item_source.count(clear_anchor) != 1:
    raise SystemExit(
        f"issue-1598 clear anchor count: {item_source.count(clear_anchor)}"
    )
item_path.write_text(
    item_source.replace(clear_anchor, clear_insert, 1), encoding="utf-8"
)

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

    void test_seed_14_prefers_prized_treasure_route() {
      const auto scenario = sim::scenario_by_label("strict-jit/go-first");
      const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
      expect(scenario.has_value() && deck != nullptr,
             "The issue-1598 fixture is unavailable.");

      std::mt19937_64 rng{14};
      sim::TraceLog trace{true, {}};
      sim::Engine engine(*scenario, deck->recipe, rng, &trace);
      const sim::TrialOutcome outcome = engine.run();

      // Gladion reveals Mysterious Treasure. The held Fire guarantees next-turn GGF,
      // and Treasure may discard Mega Dragonite ex while searching Regidrago VSTAR:
      // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2-166
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1598
      expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
             "Seed 14 did not reach strict-JIT readiness on turn three.");
      expect(trace_contains(trace, "exchanged Gladion for Mysterious Treasure") &&
                 !trace_contains(trace, "T2 | DISCARD | rules: R-MT-01") &&
                 trace_contains(trace, "T3 | DISCARD | rules: R-MT-01 | Mega Dragonite ex") &&
                 trace_contains(trace, "T3 | EVOLVE") &&
                 trace_contains(trace, "T3 | READY"),
             "Seed 14 did not execute the banked prized-Treasure route.");
    }
    }

    int main() {
      try {
        test_seed_14_prefers_prized_treasure_route();
        std::cout << "Issue 1598 prized-Treasure tests passed\n";
        return 0;
      } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
      }
    }
    '''
).lstrip()
Path("tests/issue_1598_prized_treasure_route_tests.cpp").write_text(
    test_source, encoding="utf-8"
)
