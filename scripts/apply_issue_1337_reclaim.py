from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1337"
SOURCE_PATH = Path("src/trace_engine_v2/part_search_item_overrides.inc")
LOCK_PATH = SOURCE_PATH.with_suffix(SOURCE_PATH.suffix + ".issue1337.lock")


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
            "    // https://api.pokemontcg.io/v2/cards/sv6-130",
            "    // https://api.pokemontcg.io/v2/cards/swsh12-136",
            "    // https://www.pokemon.com/us/pokemon-tcg/rules",
            "    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities",
            "    // https://github.com/FlareZ123/pokemon-sims/issues/1337",
            "    const bool payload_already_held =",
            "        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);",
            "    const bool want_payload = permit_payload && can_play_payload_this_turn() && need_payload() &&",
            "                              !payload_already_held && hand_payload_discard_outlet;",
        ]
    )

    with locked_file(LOCK_PATH):
        text = SOURCE_PATH.read_text(encoding="utf-8")
        if ISSUE_URL in text:
            return 0
        if text.count(old) != 1:
            raise RuntimeError("Active Evolution Incense selector changed unexpectedly")
        atomic_write(SOURCE_PATH, text.replace(old, new, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
