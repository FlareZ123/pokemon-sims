from __future__ import annotations

import os
from pathlib import Path

FSS = Path("src/trace_engine_v2/part_010_fss_override.inc")
QB = Path("src/trace_engine_v2/part_009b1.inc")


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {label} count: {count}")
    return text.replace(old, new, 1)


def main() -> int:
    fss = FSS.read_text(encoding="utf-8")
    fss = replace_once(
        fss,
        "        deck_count_after_search_started(Card::LatiasEx) == 0 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||",
        "FSS Latias count",
    )
    fss = replace_once(
        fss,
        "        deck_count_after_search_started(Card::Grass) < 2 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2 ||",
        "FSS Grass count",
    )
    fss = replace_once(
        fss,
        "        deck_count_after_search_started(Card::Fire) == 0 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::Fire) == 0 ||",
        "FSS Fire count",
    )

    qb = QB.read_text(encoding="utf-8")
    qb = replace_once(
        qb,
        "        deck_count_after_search_started(Card::LatiasEx) == 0 ||",
        "        std::count(state_.deck.begin(), state_.deck.end(), Card::LatiasEx) == 0 ||",
        "Quick Ball Latias count",
    )

    atomic_write(FSS, fss)
    atomic_write(QB, qb)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
