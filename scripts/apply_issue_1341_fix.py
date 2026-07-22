from __future__ import annotations

from pathlib import Path


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1341"


def main() -> int:
    path = Path("src/trace_engine_v2/part_014b.inc")
    text = path.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return 0

    old = "\n".join(
        [
            "    // Brilliant Blender is an Item that searches the deck and discards the current-turn payload: https://api.pokemontcg.io/v2/cards/sv8-164",
            "    // Preserve Burnet's one Supporter play while that live Item line supplies the same payload axis: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://www.pokemon.com/us/pokemon-tcg/rules",
            "    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && !has_live_blender_payload_line() && play_professor_burnet()) return;",
        ]
    )
    new = "\n".join(
        [
            "    // Brilliant Blender is an Item that searches the deck and discards the current-turn payload: https://api.pokemontcg.io/v2/cards/sv8-164",
            "    // Preserve Burnet's one Supporter play while another public hand route already",
            "    // supplies the same payload axis:",
            "    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // https://api.pokemontcg.io/v2/cards/sv4-163",
            "    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146",
            "    // https://api.pokemontcg.io/v2/cards/swsh12-164",
            "    // https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/888",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/942",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1341",
            "    const bool held_payload_outlet =",
            "        has_live_one_discard_hand_payload_line() ||",
            "        has_live_ultra_ball_hand_payload_line() ||",
            "        has_live_serena_hand_payload_line();",
            "    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 &&",
            "        !has_live_blender_payload_line() && !held_payload_outlet &&",
            "        play_professor_burnet()) return;",
        ]
    )
    if text.count(old) != 1:
        raise RuntimeError("Professor Burnet selector changed unexpectedly")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
