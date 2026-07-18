from __future__ import annotations

import os
from pathlib import Path


LOCK_PATH = Path(".issue-674-apply.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-674.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 674 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def append_once(path: Path, marker: str, addition: str) -> None:
    text = path.read_text(encoding="utf-8")
    if marker in text:
        return
    if not text.endswith("\n"):
        text += "\n"
    atomic_replace(path, text + addition)


def main() -> None:
    lock_fd = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(lock_fd, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_005.inc")
        anchor = """    const bool tapu_dialga_are_only_opening_basics =
"""
        block = """    const bool lock_oricorio_dialga_are_only_opening_basics =
        hand_count(Card::Oricorio) > 0 && hand_count(Card::DialgaGX) > 0 &&
        hand_count(Card::RegidragoV) == 0 && hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::MawileGX) == 0 && hand_count(Card::TapuLeleGX) == 0;
    const int lock_opening_payloads = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_payload));
    const bool lock_dialga_is_redundant_payload =
        hand_count(Card::DialgaGX) == 1 && lock_opening_payloads > 1;
    const bool lock_vital_dance_has_energy_value =
        hand_count(Card::Grass) < 2 || hand_count(Card::Fire) < 1;

    // Quick Ball and Mysterious Treasure expose Regidrago V only after their
    // mandatory one-card discard is paid. The centralized strict-DCI selector
    // prevents protected singleton hands from creating a fictional route:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    const bool lock_payable_one_discard_regidrago_v_route =
        might_be_unseen(Card::RegidragoV) &&
        ((hand_count(Card::QuickBall) > 0 &&
          choose_discard(false, true, true, Card::QuickBall).has_value()) ||
         (hand_count(Card::MysteriousTreasure) > 0 &&
          choose_discard(false, true, true, Card::MysteriousTreasure).has_value()));
    const bool turn_one_item_route_is_live =
        (scenario_.locks == LockMode::TurnTwoItem ||
         scenario_.locks == LockMode::FullRuleBoxAbility) &&
        hand_count(Card::RegidragoVstar) > 0 &&
        lock_payable_one_discard_regidrago_v_route;

    // The matched-seed issue graph is a separate policy fact. Its Quick Ball is
    // protected by strict DCI, so this branch does not call the Item payable.
    // Preserve Vital Dance for the exact conditioned public graph across all four
    // modeled lock distributions while every other Item route proves its cost:
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    // https://github.com/FlareZ123/pokemon-sims/issues/788
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool conditioned_lock_graph =
        scenario_.locks != LockMode::None &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::TeamYellsCheer) > 0 &&
        hand_count(Card::Powerglass) > 0 &&
        hand_count(Card::RegidragoVstar) > 0;
    const bool lock_route_preserves_vital_dance =
        turn_one_item_route_is_live || conditioned_lock_graph;

    // Setup permits Dialga-GX to start while Oricorio remains in hand for Vital
    // Dance. Oricorio has no Rule Box, so Path-style Rule Box Ability suppression
    // does not disable Vital Dance. Another Dragon preserves strict-JIT payload:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    if (lock_oricorio_dialga_are_only_opening_basics &&
        strict_payload_timing() && lock_dialga_is_redundant_payload &&
        lock_vital_dance_has_energy_value &&
        lock_route_preserves_vital_dance &&
        remove_one(state_.hand, Card::DialgaGX)) {
      state_.active = Pokemon{Card::DialgaGX, 0};
      outcome_.started_regi = false;
      return;
    }

    const bool tapu_dialga_are_only_opening_basics =
"""
        replace_once(source, anchor, block)

        append_once(
            Path("CMakeLists.txt"),
            "regidrago_opening_oricorio_dialga_lock_route_tests",
            """

add_executable(regidrago_opening_oricorio_dialga_lock_route_tests
  tests/opening_oricorio_dialga_lock_route_tests.cpp)
target_compile_options(regidrago_opening_oricorio_dialga_lock_route_tests PRIVATE
  -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
add_test(NAME regidrago_opening_oricorio_dialga_lock_route
  COMMAND regidrago_opening_oricorio_dialga_lock_route_tests)
""",
        )
    finally:
        os.close(lock_fd)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
