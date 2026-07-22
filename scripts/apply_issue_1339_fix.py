from __future__ import annotations

from pathlib import Path


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1339"


def main() -> int:
    path = Path("src/trace_engine_v2/part_010_blender_thinning_override.inc")
    text = path.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return 0

    old = "\n".join(
        [
            "    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||",
            "        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||",
            "        !blender_energy_axis_can_finish_this_turn() ||",
            "        !blender_active_axis_can_finish_this_turn()) {",
            "      return false;",
            "    }",
        ]
    )
    new = "\n".join(
        [
            old,
            "",
            "    // A public Dragon payload plus a legal hand-discard outlet already completes",
            "    // the current-turn payload axis. Preserve the one-copy ACE SPEC and the deck",
            "    // payloads instead of duplicating that role:",
            "    // https://api.pokemontcg.io/v2/cards/sv8-164",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/me2pt5-152",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/976",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1339",
            "    const bool held_payload_outlet =",
            "        has_live_one_discard_hand_payload_line() ||",
            "        has_live_ultra_ball_hand_payload_line() ||",
            "        has_live_serena_hand_payload_line();",
            "    if (held_payload_outlet) return false;",
        ]
    )
    if text.count(old) != 1:
        raise RuntimeError("Active Brilliant Blender selector changed unexpectedly")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
