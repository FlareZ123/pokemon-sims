from __future__ import annotations

import os
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


SOURCE_PATH = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
LOCK_PATH = SOURCE_PATH.with_suffix(SOURCE_PATH.suffix + ".issue1389.lock")


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
    old = """      // With VSTAR already held, the Box's independent Item category can take
      // Quick Ball to discard the Dawn-supplied Dragon. A held Treasure is then
      // a replaceable cost rather than an irreplaceable connector.
      if (card == Card::MysteriousTreasure &&
          (has_vstar() || hand_count(Card::RegidragoVstar) > 0) &&
          (hand_count(Card::QuickBall) > 0 || might_be_unseen(Card::QuickBall)) &&
          (payload_without_dawn || hand_count(Card::Dawn) > 0 ||
           might_be_unseen(Card::Dawn))) return 7;
"""
    new = """      // With VSTAR already held, the Box's independent Item category can take
      // Quick Ball to discard the Dawn-supplied Dragon. The held Treasure is
      // replaceable, though it remains more valuable than an Earthen Vessel
      // whose Fire axis is already covered by the same proven combo:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://github.com/FlareZ123/pokemon-sims/issues/1389
      if (card == Card::MysteriousTreasure &&
          (has_vstar() || hand_count(Card::RegidragoVstar) > 0) &&
          (hand_count(Card::QuickBall) > 0 || might_be_unseen(Card::QuickBall)) &&
          (payload_without_dawn || hand_count(Card::Dawn) > 0 ||
           might_be_unseen(Card::Dawn))) return 8;
"""

    with locked_file(LOCK_PATH):
        source = SOURCE_PATH.read_text(encoding="utf-8")
        if new in source:
            return 0
        if source.count(old) != 1:
            raise RuntimeError(
                f"Unexpected issue-1389 source anchor count: {source.count(old)}"
            )
        atomic_write(SOURCE_PATH, source.replace(old, new, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
