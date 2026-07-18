from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-781-v2.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-781-v2.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 781 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        source = Path("src/trace_engine_v2/part_012_override.inc")
        old_setup = '''    const bool can_pay_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);

    const auto item_candidates_for_current_knowledge = [this, can_pay_search_cost]() {
'''
        new_setup = '''    const bool can_pay_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);

    const auto held_treasure_then_quick_ball_is_payable = [this]() {
      if (item_locked() || hand_count(Card::MysteriousTreasure) == 0 ||
          hand_count(Card::QuickBall) > 0 || !might_be_unseen(Card::RegidragoVstar) ||
          !might_be_unseen(Card::QuickBall)) return false;
      const std::vector<Card> original_hand = state_.hand;
      const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
      const bool permit_payload = can_play_payload_this_turn();
      const auto treasure_cost = choose_discard(permit_payload, true, true, Card::MysteriousTreasure);
      if (!treasure_cost || !remove_one(state_.hand, Card::MysteriousTreasure) ||
          !remove_one(state_.hand, *treasure_cost)) {
        restore_hand();
        return false;
      }
      state_.hand.push_back(Card::RegidragoVstar);
      state_.hand.push_back(Card::QuickBall);
      const bool payable = choose_discard(permit_payload, true, true, Card::QuickBall).has_value();
      restore_hand();
      return payable;
    };
    const bool evolvable_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
        });
    const auto complementary_quick_ball_for_latias = [this, evolvable_benched_regi,
                                                         &held_treasure_then_quick_ball_is_payable]() {
      return need_vstar() && state_.active && state_.active->card != Card::RegidragoV &&
             state_.active->card != Card::RegidragoVstar && evolvable_benched_regi &&
             bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
             hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
             might_be_unseen(Card::LatiasEx) && held_treasure_then_quick_ball_is_payable();
    };

    const auto item_candidates_for_current_knowledge =
        [this, can_pay_search_cost, &complementary_quick_ball_for_latias]() {
'''
        replace_once(source, old_setup, new_setup)

        old_candidate = '''      if (known_prized_regi_target) add_item_candidate(Card::HisuianHeavyBall, true);
      if (need_vstar() && live_vstar_target) {
'''
        new_candidate = '''      if (known_prized_regi_target) add_item_candidate(Card::HisuianHeavyBall, true);
      // A held Mysterious Treasure can cover Regidrago VSTAR. Prefer the
      // complementary Quick Ball when both printed discard costs remain payable
      // and Latias ex is required to promote the evolved Benched attacker:
      // https://api.pokemontcg.io/v2/cards/sv1-166
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/781
      if (complementary_quick_ball_for_latias()) add_item_candidate(Card::QuickBall, true);
      if (need_vstar() && live_vstar_target) {
'''
        replace_once(source, old_candidate, new_candidate)

        cmake = Path("CMakeLists.txt")
        cmake_anchor = "set_tests_properties(regidrago_sim_matrix_smoke PROPERTIES RUN_SERIAL TRUE)\n"
        cmake_replacement = cmake_anchor + "\nadd_executable(regidrago_arven_complementary_latias_tests tests/arven_complementary_latias_route_tests.cpp)\ntarget_compile_options(regidrago_arven_complementary_latias_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\nadd_test(NAME regidrago_arven_complementary_latias COMMAND regidrago_arven_complementary_latias_tests)\n"
        replace_once(cmake, cmake_anchor, cmake_replacement)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
