from __future__ import annotations

from pathlib import Path


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1343"


def main() -> int:
    path = Path("src/trace_engine_v2/part_012_override.inc")
    text = path.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return 0

    old = "\n".join(
        [
            "    const std::vector<Card> pre_search_candidates = item_candidates_for_current_knowledge();",
            "    const bool pre_search_live_fss = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);",
            "    const bool pre_search_live_powerglass = powerglass_has_live_target();",
            "    if (pre_search_candidates.empty() && !pre_search_live_fss && !pre_search_live_powerglass) return false;",
        ]
    )
    new = "\n".join(
        [
            "    const std::vector<Card> pre_search_candidates = item_candidates_for_current_knowledge();",
            "    const bool pre_search_live_fss = needs_tool && fss_has_live_target() && might_be_unseen(Card::ForestSealStone);",
            "    const bool pre_search_live_powerglass = powerglass_has_live_target();",
            "",
            "    // A public held Dragon plus a target-legal discard Item already completes",
            "    // strict JIT when payload is the only missing axis. Preserve the Supporter,",
            "    // ACE SPEC, and deck payloads instead of using Arven to manufacture a",
            "    // redundant Brilliant Blender route:",
            "    // https://api.pokemontcg.io/v2/cards/sv1-166",
            "    // https://api.pokemontcg.io/v2/cards/sv8-164",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // https://api.pokemontcg.io/v2/cards/sv4-163",
            "    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146",
            "    // https://api.pokemontcg.io/v2/cards/me2pt5-152",
            "    // https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1343",
            "    const bool held_payload_outlet =",
            "        has_live_one_discard_hand_payload_line() ||",
            "        has_live_ultra_ball_hand_payload_line();",
            "    const bool payload_is_only_missing_axis = need_payload() && !need_regi() &&",
            "        !need_vstar() && !need_energy() && !need_active_vstar();",
            "    if (payload_is_only_missing_axis && held_payload_outlet) return false;",
            "",
            "    if (pre_search_candidates.empty() && !pre_search_live_fss && !pre_search_live_powerglass) return false;",
        ]
    )
    if text.count(old) != 1:
        raise RuntimeError("Arven pre-search block changed unexpectedly")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
