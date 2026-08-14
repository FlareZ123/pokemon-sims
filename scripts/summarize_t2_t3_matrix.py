from __future__ import annotations

import argparse
import csv
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


REQUIRED_COLUMNS = (
    "deck",
    "scenario",
    "ready_by_t2_pct",
    "ready_by_t3_pct",
)


@contextmanager
def exclusive_lock(path: Path):
    path.parent.mkdir(parents=True, exist_ok=True)
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        yield
    finally:
        os.close(descriptor)
        path.unlink(missing_ok=True)


def atomic_write_text(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="", dir=path.parent, delete=False
    ) as handle:
        handle.write(text)
        temporary_path = Path(handle.name)
    os.replace(temporary_path, path)


def render_summary(rows: list[dict[str, str]]) -> str:
    lines = [
        "# Generated T2/T3 setup summary\n",
        "\n",
        "| Deck | Scenario | Ready by T2 | Ready by T3 |\n",
        "|---|---|---:|---:|\n",
    ]
    for row in rows:
        lines.append(
            f"| {row['deck']} | {row['scenario']} | "
            f"{float(row['ready_by_t2_pct']):.3f}% | "
            f"{float(row['ready_by_t3_pct']):.3f}% |\n"
        )
    return "".join(lines)


def summarize_matrix(source: Path, destination: Path) -> None:
    with source.open("r", encoding="utf-8", newline="") as handle:
        reader = csv.DictReader(handle)
        fieldnames = tuple(reader.fieldnames or ())
        missing = [column for column in REQUIRED_COLUMNS if column not in fieldnames]
        if missing:
            raise ValueError(f"matrix is missing required columns {missing}: {source}")
        rows = list(reader)
    if not rows:
        raise ValueError(f"empty matrix: {source}")

    summary = render_summary(rows)
    lock_path = Path(f"{destination}.lock")
    with exclusive_lock(lock_path):
        atomic_write_text(destination, summary)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Render a canonical human-readable T2/T3 table from an aggregate matrix."
    )
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    summarize_matrix(args.input, args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
