from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-788-current.lock")
SOURCE = Path("src/trace_engine_v2/part_005.inc")
HELPER = '''

  bool move_deck_to_discard(const Card card) {
    record_deck_search_knowledge("search effect");
    if (!remove_one(state_.deck, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    return true;
  }
'''


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        text = SOURCE.read_text(encoding="utf-8")
        if "bool move_deck_to_discard(const Card card)" in text:
            return
        temporary = SOURCE.with_name(f".{SOURCE.name}.issue-788.tmp")
        temporary.write_text(text.rstrip() + HELPER, encoding="utf-8", newline="\n")
        os.replace(temporary, SOURCE)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
