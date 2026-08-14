from __future__ import annotations

import argparse
import csv
import math
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


READY_BY_COLUMNS = (
    "ready_by_t2_pct",
    "ready_by_t3_pct",
    "ready_by_t4_pct",
    "ready_by_t5_pct",
)

# This is a catastrophic-skew tripwire, not a replacement for the exact
# source-bound matrix comparison. The current registered-deck 100k population is
# below 60% ready by T2 and below 74% ready by T3; these deliberately broad
# ceilings leave material headroom while rejecting the 88-99% class of aggregate
# inflation that triggered issue #3761.
# Canonical matrix contract:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
# Confirmed statistical-sanity defect:
# https://github.com/FlareZ123/pokemon-sims/issues/3761
EARLY_READY_CEILINGS = {
    "ready_by_t2_pct": 70.0,
    "ready_by_t3_pct": 85.0,
}


# The paired aggregate already contains both registered decks, including the
# canonical regidrago-shell rows, so those rows can be reused byte-for-byte:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
# https://github.com/FlareZ123/pokemon-sims/issues/2724
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


def parsed_row(line: str) -> list[str]:
    return next(csv.reader([line]))


def validate_matrix_statistics(header: list[str], lines: list[str], source: Path) -> None:
    missing_columns = [column for column in READY_BY_COLUMNS if column not in header]
    if missing_columns:
        raise ValueError(
            f"matrix is missing cumulative readiness columns {missing_columns}: {source}"
        )

    for line in lines:
        values = parsed_row(line)
        if len(values) != len(header):
            raise ValueError(
                f"matrix row has {len(values)} fields but header has {len(header)}: {source}"
            )
        row = dict(zip(header, values))
        deck = row.get("deck", "<unknown-deck>")
        scenario = row.get("scenario", "<unknown-scenario>")

        readiness: list[float] = []
        for column in READY_BY_COLUMNS:
            try:
                value = float(row[column])
            except ValueError as error:
                raise ValueError(
                    f"invalid readiness percentage for {deck}/{scenario} "
                    f"{column}: {row[column]!r}"
                ) from error
            if not math.isfinite(value) or value < 0.0 or value > 100.0:
                raise ValueError(
                    f"readiness percentage outside 0-100 for {deck}/{scenario} "
                    f"{column}: {value}"
                )
            readiness.append(value)

        if readiness != sorted(readiness):
            raise ValueError(
                f"cumulative readiness is not monotone for {deck}/{scenario}: {readiness}"
            )

        for column, ceiling in EARLY_READY_CEILINGS.items():
            value = float(row[column])
            if value > ceiling:
                raise ValueError(
                    "egregious early-readiness skew: "
                    f"{deck}/{scenario} {column}={value:.3f}% exceeds "
                    f"{ceiling:.1f}% guardrail"
                )


def extract_deck_rows(source: Path, destination: Path, deck: str) -> None:
    with source.open("r", encoding="utf-8", newline="") as handle:
        lines = handle.readlines()
    if not lines:
        raise ValueError(f"empty matrix: {source}")

    header = parsed_row(lines[0])
    if not header or header[0] != "deck":
        raise ValueError(f"matrix does not start with a deck column: {source}")

    # CI derives the canonical shell matrix from this paired aggregate. Validate the
    # complete paired population first so either registered deck can trip the guard.
    validate_matrix_statistics(header, lines[1:], source)

    selected = [line for line in lines[1:] if parsed_row(line)[0] == deck]
    if not selected:
        raise ValueError(f"deck not found in matrix: {deck}")

    lock_path = Path(f"{destination}.lock")
    with exclusive_lock(lock_path):
        atomic_write_text(destination, lines[0] + "".join(selected))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Extract one deck's rows from an already-generated paired matrix."
    )
    parser.add_argument("--input", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--deck", required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    extract_deck_rows(args.input, args.output, args.deck)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
