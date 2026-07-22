from __future__ import annotations

from pathlib import Path


# This idempotent helper keeps the confirmed source patch reproducible in CI.
ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1335"


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return
    if old not in text:
        raise RuntimeError(f"Expected selector block was not found in {path}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")


def main() -> int:
    treasure_path = Path("src/trace_engine_v2/part_009a.inc")
    treasure_old = "\n".join(
        [
            "    const bool want_payload = need_payload() && !can_jit_discard &&",
            "                              !has_live_serena_hand_payload_line() &&",
            "                              has_payable_payload_outlet_after_costed_search(Card::MysteriousTreasure);",
        ]
    )
    treasure_new = "\n".join(
        [
            "    // A public Dragon payload can pay the surviving hand-discard outlet directly.",
            "    // Fetching another payload first consumes Mysterious Treasure plus an extra cost",
            "    // without advancing the current ready turn:",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // https://api.pokemontcg.io/v2/cards/me2pt5-152",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1335",
            "    const bool payload_already_held =",
            "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);",
            "    const bool want_payload = need_payload() && !can_jit_discard &&",
            "                              !payload_already_held &&",
            "                              !has_live_serena_hand_payload_line() &&",
            "                              has_payable_payload_outlet_after_costed_search(Card::MysteriousTreasure);",
        ]
    )
    replace_once(treasure_path, treasure_old, treasure_new)

    quick_path = Path("src/trace_engine_v2/part_009b1.inc")
    quick_old = "\n".join(
        [
            "    const bool want_payload = need_payload() && might_be_unseen(Card::DialgaGX) && !can_pay_payload_cost &&",
            "                              has_payable_payload_outlet_after_costed_search(Card::QuickBall);",
        ]
    )
    quick_new = "\n".join(
        [
            "    // When a Dragon payload is already held, the surviving hand-discard outlet can",
            "    // spend that public payload directly. Fetching a second Dialga-GX first consumes",
            "    // Quick Ball and an extra discard without advancing readiness:",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-179",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/sm5-100",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1335",
            "    const bool payload_already_held =",
            "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);",
            "    const bool want_payload = need_payload() && !payload_already_held &&",
            "                              might_be_unseen(Card::DialgaGX) && !can_pay_payload_cost &&",
            "                              has_payable_payload_outlet_after_costed_search(Card::QuickBall);",
        ]
    )
    replace_once(quick_path, quick_old, quick_new)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
