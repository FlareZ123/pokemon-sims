from __future__ import annotations

import argparse
import csv
from decimal import Decimal, InvalidOperation
from pathlib import Path

from extract_deck_matrix import atomic_write_text, exclusive_lock


# Read the authoritative paired aggregate by column name so human reporting cannot
# drift from the CSV schema or rely on positional/manual transcription:
# https://github.com/FlareZ123/pokemon-sims/issues/3764
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
REQUIRED_COLUMNS = ("deck", "scenario", "ready_by_t2_pct", "ready_by_t3_pct")
REQUIRED_DECKS = frozenset(("regidrago-shell", "regidrago-pineco"))


def format_percent(value: str, column: str) -> str:
    try:
        percentage = Decimal(value)
    except InvalidOperation as exc:
        raise ValueError(f"invalid {column} percentage: {value!r}") from exc
    return f"{percentage:.3f}%"


def render_summary(source: Path) -> str:
    with source.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = reader.fieldnames or []
        missing_columns = [name for name in REQUIRED_COLUMNS if name not in fieldnames]
        if missing_columns:
            raise ValueError(
                "matrix is missing required columns: " + ", ".join(missing_columns)
            )
        rows = list(reader)

    if not rows:
        raise ValueError(f"empty matrix: {source}")

    decks = {row["deck"] for row in rows}
    missing_decks = sorted(REQUIRED_DECKS - decks)
    if missing_decks:
        raise ValueError("matrix is missing required decks: " + ", ".join(missing_decks))

    lines = [
        "# T2/T3 Setup Probability Summary",
        "",
        f"Source: `{source.name}`",
        "",
        "| Deck | Scenario | Ready by T2 | Ready by T3 |",
        "| --- | --- | ---: | ---: |",
    ]
    for row in rows:
        lines.append(
            "| "
            + " | ".join(
                (
                    row["deck"],
                    row["scenario"],
                    format_percent(row["ready_by_t2_pct"], "ready_by_t2_pct"),
                    format_percent(row["ready_by_t3_pct"], "ready_by_t3_pct"),
                )
            )
            + " |"
        )
    return "\n".join(lines) + "\n"


def write_summary(source: Path, destination: Path) -> None:
    summary = render_summary(source)
    lock_path = Path(f"{destination}.lock")
    with exclusive_lock(lock_path):
        atomic_write_text(destination, summary)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a deterministic readable T2/T3 summary from a paired matrix."
    )
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    write_summary(args.input, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
