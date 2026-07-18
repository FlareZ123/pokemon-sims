from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-775-v2.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-775-v2.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 775 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_006.inc")
        old_selector = '''    if (hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) >= 2) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && has_vstar()) return Card::RegidragoVstar;

    // Extra in-hand Energy is observable; no deck / Prize identities are used to call Energy expendable.
'''
        new_selector = '''    if (hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) >= 2) return Card::RegidragoV;
    if (hand_count(Card::RegidragoVstar) > 0 && has_vstar()) return Card::RegidragoVstar;

    const bool setup_before_steven_route =
        excluded_from_cost && *excluded_from_cost == Card::MysteriousTreasure &&
        state_.turn == 1 && !scenario_.going_first && !item_locked() &&
        supporter_allowed() && need_regi() && bench_space() > 0 &&
        can_free_retreat_with_latias() && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::Fire) == 1 && hand_count(Card::Grass) > 0 &&
        might_be_unseen(Card::RegidragoV) && might_be_unseen(Card::RegidragoVstar) &&
        might_be_unseen(Card::Crispin) && might_be_unseen(Card::BrilliantBlender) &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire);
    if (setup_before_steven_route) {
      // Spending the public Fire Energy lets Mysterious Treasure establish
      // Regidrago V before Steven's Resolve ends turn one. Grass remains for the
      // manual attachment, Crispin restores GGF on turn two, and Latias ex supplies
      // the proven promotion route:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sv8-76
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/775
      return Card::Fire;
    }

    // Extra in-hand Energy is observable; no deck / Prize identities are used to call Energy expendable.
'''
        replace_once(source, old_selector, new_selector)

        cmake = Path("CMakeLists.txt")
        target_anchor = '''add_executable(regidrago_steven_missing_regi_tests tests/steven_missing_regi_tests.cpp)
target_compile_options(regidrago_steven_missing_regi_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        target_replacement = target_anchor + '''
add_executable(regidrago_steven_mysterious_treasure_dci_tests tests/steven_mysterious_treasure_dci_tests.cpp)
target_compile_options(regidrago_steven_mysterious_treasure_dci_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        replace_once(cmake, target_anchor, target_replacement)

        test_anchor = "add_test(NAME regidrago_steven_missing_regi COMMAND regidrago_steven_missing_regi_tests)\n"
        test_replacement = test_anchor + "add_test(NAME regidrago_steven_mysterious_treasure_dci COMMAND regidrago_steven_mysterious_treasure_dci_tests)\n"
        replace_once(cmake, test_anchor, test_replacement)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
