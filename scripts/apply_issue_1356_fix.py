from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1356"
SOURCE_PATH = Path("src/trace_engine_v2/part_010_fss_override.inc")
LOCK_PATH = SOURCE_PATH.with_suffix(SOURCE_PATH.suffix + ".lock")


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def main() -> int:
    helper_anchor = "\n  Card fss_target_after_search_started() const {"
    target_anchor = "\n".join(
        [
            "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;",
            "",
            "    const Card original = fss_target_after_search_started_original();",
        ]
    )
    helper_block = "\n".join(
        [
            "  bool fss_should_split_treasure_vstar_and_next_turn_energy() const {",
            "    if (!deck_seen_ || state_.vstar_power_used || item_locked() ||",
            "        !strict_payload_timing() || state_.turn >= scenario_.max_turn ||",
            "        !state_.manual_energy_used || !need_vstar() || !need_energy() ||",
            "        grass_needed() + fire_needed() != 1 || !state_.active ||",
            "        state_.active->card != Card::RegidragoV ||",
            "        state_.active->entered_turn >= state_.turn ||",
            "        hand_count(Card::MysteriousTreasure) < 2 ||",
            "        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||",
            "        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {",
            "      return false;",
            "    }",
            "",
            "    const bool missing_energy_available =",
            "        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||",
            "        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);",
            "    if (!missing_energy_available) return false;",
            "",
            "    // A payable held Mysterious Treasure can search the missing Regidrago VSTAR,",
            "    // while Star Alchemy is the costless connector for the one missing Basic Energy",
            "    // that must be attached next turn. Project the Treasure play through the real DCI",
            "    // selector and require one Treasure plus the held Dragon payload to survive:",
            "    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166",
            "    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156",
            "    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // Official turn, Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // Route priority and strict-JIT DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356",
            "    const std::size_t held_payloads_before = static_cast<std::size_t>(std::count_if(",
            "        state_.hand.begin(), state_.hand.end(), is_payload));",
            "    std::mt19937_64 shadow_rng = rng_;",
            "    Engine projected(scenario_, recipe_, shadow_rng);",
            "    projected.state_ = state_;",
            "    projected.deck_seen_ = deck_seen_;",
            "    projected.prizes_revealed_ = prizes_revealed_;",
            "    if (!projected.play_mysterious_treasure(false)) return false;",
            "    const std::size_t held_payloads_after = static_cast<std::size_t>(std::count_if(",
            "        projected.state_.hand.begin(), projected.state_.hand.end(), is_payload));",
            "    return projected.hand_count(Card::RegidragoVstar) > 0 &&",
            "        projected.hand_count(Card::MysteriousTreasure) > 0 &&",
            "        held_payloads_after == held_payloads_before;",
            "  }",
            "",
        ]
    )
    target_block = "\n".join(
        [
            "    if (fss_should_take_crispin_over_redundant_blender()) return Card::Crispin;",
            "",
            "    if (fss_should_split_treasure_vstar_and_next_turn_energy()) {",
            "      // Star Alchemy searches any card. Treasure supplies the Dragon VSTAR axis,",
            "      // so preserve Quick Ball, Oricorio, the Bench slot, and the second Treasure",
            "      // by assigning Forest Seal Stone to the missing next-turn Basic Energy:",
            "      // https://api.pokemontcg.io/v2/cards/swsh12-156",
            "      // https://api.pokemontcg.io/v2/cards/sm6-113",
            "      // https://api.pokemontcg.io/v2/cards/sm2-55",
            "      // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "      // https://github.com/FlareZ123/pokemon-sims/issues/1356",
            "      if (grass_needed() == 1) return Card::Grass;",
            "      return Card::Fire;",
            "    }",
            "",
            "    const Card original = fss_target_after_search_started_original();",
        ]
    )

    with locked_file(LOCK_PATH):
        text = SOURCE_PATH.read_text(encoding="utf-8")
        if ISSUE_URL in text:
            return 0
        if text.count(helper_anchor) != 1:
            raise RuntimeError("Forest Seal Stone override helper anchor changed unexpectedly")
        if text.count(target_anchor) != 1:
            raise RuntimeError("Forest Seal Stone target anchor changed unexpectedly")
        text = text.replace(helper_anchor, "\n" + helper_block + helper_anchor.lstrip("\n"), 1)
        text = text.replace(target_anchor, target_block, 1)
        atomic_write(SOURCE_PATH, text)
    LOCK_PATH.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
