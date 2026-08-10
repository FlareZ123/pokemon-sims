from __future__ import annotations

import argparse
import csv
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


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


def extract_deck_rows(source: Path, destination: Path, deck: str) -> None:
    lines = source.read_text(encoding="utf-8").splitlines(keepends=True)
    if not lines:
        raise ValueError(f"empty matrix: {source}")

    header = parsed_row(lines[0])
    if not header or header[0] != "deck":
        raise ValueError(f"matrix does not start with a deck column: {source}")

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
