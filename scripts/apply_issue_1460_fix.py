#!/usr/bin/env python3
"""Apply the confirmed issue-1460 production fix atomically."""
from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

SOURCE = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
LOCK = Path(".git/issue-1460-source.lock")

OLD = '''  record_deck_search_knowledge("Exploding Energy");
  const int available = std::min(5, deck_count_after_search_started(Card::Grass));
  if (available <= 0) return false;
'''

NEW = '''  record_deck_search_knowledge("Exploding Energy");
  const int available = std::min(5, deck_count_after_search_started(Card::Grass));
  if (available <= 0) {
    // A K0 announcement can legally reveal that no Basic Grass Energy remains.
    // Once the hidden-zone search has begun, finish the printed consequence
    // instead of retaining K1 knowledge while leaving Forretress ex in play:
    // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Core deck-search, shuffle, Knock Out, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1460
    return resolve_exploding_energy({});
  }
'''


def write_atomic(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as temporary:
        temporary.write(content)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, path)


def main() -> int:
    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with LOCK.open("w", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        source = SOURCE.read_text(encoding="utf-8")
        if NEW in source:
            return 0
        if source.count(OLD) != 1:
            raise RuntimeError("issue-1460 source anchor was missing or ambiguous")
        write_atomic(SOURCE, source.replace(OLD, NEW, 1))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
