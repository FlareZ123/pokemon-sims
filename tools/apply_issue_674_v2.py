from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-674-v2.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-674-v2.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 674 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_005.inc")
        anchor = "    const bool tapu_dialga_are_only_opening_basics =\n"
        replacement = '''    const bool oricorio_dialga_are_only_opening_basics =
        hand_count(Card::Oricorio) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::TapuLeleGX) == 0;
    const int opening_payloads = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_payload));
    const bool another_payload_covers_strict_jit =
        hand_count(Card::DialgaGX) == 1 && opening_payloads > 1;
    const bool vital_dance_has_energy_value =
        hand_count(Card::Grass) < 2 || hand_count(Card::Fire) < 1;
    const bool turn_one_item_search_is_live =
        scenario_.locks == LockMode::TurnTwoItem ||
        scenario_.locks == LockMode::FullRuleBoxAbility;
    const bool full_item_conditioned_graph =
        (scenario_.locks == LockMode::FullItem ||
         scenario_.locks == LockMode::FullCombined) &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::TeamYellsCheer) > 0 &&
        hand_count(Card::Powerglass) > 0 && hand_count(Card::RegidragoVstar) > 0;
    const bool lock_route_supports_vital_dance =
        (turn_one_item_search_is_live && direct_pokemon_search_already_held) ||
        full_item_conditioned_graph;

    // Under the modeled lock scenarios, start Dialga-GX only when another held
    // Dragon already covers strict-JIT payload and preserving Oricorio keeps a live
    // Vital Dance Energy connector. Turn-two Item lock and Rule Box Ability lock
    // retain the public Item-search route. Oricorio has no Rule Box, so Path-style
    // suppression does not disable Vital Dance. The full-Item cases are restricted
    // to the approved conditioned public graph whose matched-seed policy comparison
    // favors Dialga Active despite Quick Ball being locked:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-scenarios
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    if (oricorio_dialga_are_only_opening_basics && strict_payload_timing() &&
        another_payload_covers_strict_jit && vital_dance_has_energy_value &&
        lock_route_supports_vital_dance && remove_one(state_.hand, Card::DialgaGX)) {
      state_.active = Pokemon{Card::DialgaGX, 0};
      outcome_.started_regi = false;
      return;
    }

    const bool tapu_dialga_are_only_opening_basics =
'''
        replace_once(source, anchor, replacement)

        cmake = Path("CMakeLists.txt")
        target_anchor = '''add_executable(regidrago_opening_dialga_payload_tests tests/opening_active_dialga_payload_tests.cpp)
target_compile_options(regidrago_opening_dialga_payload_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        target_replacement = target_anchor + '''
add_executable(regidrago_opening_oricorio_dialga_lock_tests tests/opening_oricorio_dialga_lock_tests.cpp)
target_compile_options(regidrago_opening_oricorio_dialga_lock_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
        replace_once(cmake, target_anchor, target_replacement)

        test_anchor = "add_test(NAME regidrago_opening_dialga_payload COMMAND regidrago_opening_dialga_payload_tests)\n"
        test_replacement = test_anchor + "add_test(NAME regidrago_opening_oricorio_dialga_lock COMMAND regidrago_opening_oricorio_dialga_lock_tests)\n"
        replace_once(cmake, test_anchor, test_replacement)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
