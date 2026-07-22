from __future__ import annotations

import os
import subprocess
from pathlib import Path


# This idempotent helper keeps the validated source patch reproducible in CI.
ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1337"


def sync_main_in_ci() -> None:
    if os.environ.get("GITHUB_ACTIONS") != "true":
        return
    branch = os.environ["GITHUB_HEAD_REF"]
    subprocess.run(["git", "config", "user.name", "github-actions[bot]"], check=True)
    subprocess.run(["git", "config", "user.email", "41898282+github-actions[bot]@users.noreply.github.com"], check=True)
    subprocess.run(["git", "fetch", "origin", "main"], check=True)
    subprocess.run(["git", "merge", "-X", "theirs", "--no-edit", "origin/main"], check=True)
    subprocess.run(["git", "push", "origin", f"HEAD:{branch}"], check=True)


def main() -> int:
    sync_main_in_ci()
    path = Path("src/trace_engine_v2/part_search_item_overrides.inc")
    text = path.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return 0

    old = "\n".join(
        [
            "    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&",
            "                              hand_payload_discard_outlet;",
        ]
    )
    new = "\n".join(
        [
            "    // A public Dragon payload can already pay the held discard outlet. Evolution",
            "    // Incense must not spend itself to fetch a second payload that advances no axis:",
            "    // https://api.pokemontcg.io/v2/cards/swsh1-163",
            "    // https://api.pokemontcg.io/v2/cards/sm6-113",
            "    // https://api.pokemontcg.io/v2/cards/me2pt5-152",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1337",
            "    const bool payload_already_held =",
            "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);",
            "    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&",
            "                              !payload_already_held && hand_payload_discard_outlet;",
        ]
    )
    if text.count(old) != 1:
        raise RuntimeError("Active Evolution Incense selector changed unexpectedly")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
