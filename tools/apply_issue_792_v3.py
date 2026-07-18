from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-792-v3.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-792-v3.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 792 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_005.inc")
        old_route = '''    const bool no_lock_public_regidrago_v_route =
        hand_count(Card::RegidragoVstar) > 0 &&
        (hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0 ||
         hand_count(Card::Arven) > 0 ||
         hand_count(Card::StevensResolve) > 0);
'''
        new_route = '''    // A held one-discard Item is a public Regidrago V connector only when the
    // centralized strict-DCI selector admits its mandatory cost. The card being
    // played cannot discard itself:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_006.inc
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/792
    const bool no_lock_payable_quick_ball_route =
        hand_count(Card::QuickBall) > 0 && might_be_unseen(Card::RegidragoV) &&
        choose_discard(false, true, true, Card::QuickBall).has_value();
    const bool no_lock_payable_mysterious_treasure_route =
        hand_count(Card::MysteriousTreasure) > 0 && might_be_unseen(Card::RegidragoV) &&
        choose_discard(false, true, true, Card::MysteriousTreasure).has_value();
    const bool no_lock_public_regidrago_v_route =
        hand_count(Card::RegidragoVstar) > 0 &&
        (no_lock_payable_quick_ball_route ||
         no_lock_payable_mysterious_treasure_route ||
         hand_count(Card::Arven) > 0 ||
         hand_count(Card::StevensResolve) > 0);
'''
        replace_once(source, old_route, new_route)

        existing_test = Path("tests/opening_oricorio_dialga_no_lock_tests.cpp")
        replace_once(
            existing_test,
            '''  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
''',
            '''  state.hand = {sim::Card::QuickBall, sim::Card::QuickBall,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
''',
        )
        replace_once(
            existing_test,
            "void test_exact_issue_hand_preserves_vital_dance() {\n",
            "void test_payable_issue_hand_preserves_vital_dance() {\n",
        )
        replace_once(
            existing_test,
            '''  // Setup can start Dialga-GX. Mega Dragonite ex preserves a second strict-JIT
  // payload, Quick Ball exposes Regidrago V, and held Oricorio remains a legal
  // later Vital Dance Energy connector when played from hand onto the Bench:
''',
            '''  // Setup can start Dialga-GX. Mega Dragonite ex preserves a second strict-JIT
  // payload, and one Quick Ball may discard the distinct second copy before
  // searching Regidrago V. Held Oricorio remains a later Vital Dance connector:
''',
        )
        replace_once(
            existing_test,
            '"The exact issue hand should start Dialga-GX and preserve Oricorio.");\n',
            '"A payable issue route should start Dialga-GX and preserve Oricorio.");\n',
        )
        replace_once(
            existing_test,
            "  test_exact_issue_hand_preserves_vital_dance();\n",
            "  test_payable_issue_hand_preserves_vital_dance();\n",
        )

        cmake = Path("CMakeLists.txt")
        target_anchor = '''add_executable(regidrago_opening_oricorio_dialga_no_lock_tests tests/opening_oricorio_dialga_no_lock_tests.cpp)
target_compile_options(regidrago_opening_oricorio_dialga_no_lock_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        target_replacement = target_anchor + '''
add_executable(regidrago_opening_oricorio_dialga_payability_tests tests/opening_oricorio_dialga_payability_tests.cpp)
target_compile_options(regidrago_opening_oricorio_dialga_payability_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        replace_once(cmake, target_anchor, target_replacement)

        test_anchor = "add_test(NAME regidrago_opening_oricorio_dialga_no_lock COMMAND regidrago_opening_oricorio_dialga_no_lock_tests)\n"
        test_replacement = test_anchor + "add_test(NAME regidrago_opening_oricorio_dialga_payability COMMAND regidrago_opening_oricorio_dialga_payability_tests)\n"
        replace_once(cmake, test_anchor, test_replacement)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
