from __future__ import annotations

import argparse
import csv
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


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
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as handle:
        handle.write(text)
        temporary_path = Path(handle.name)
    os.replace(temporary_path, path)


def project_deck_matrix(source: Path, output: Path, deck: str) -> None:
    lines = source.read_text(encoding="utf-8").splitlines(keepends=True)
    if not lines:
        raise ValueError(f"Matrix is empty: {source}")

    header = next(csv.reader([lines[0]]))
    if not header or header[0] != "deck":
        raise ValueError(f"Matrix must begin with a deck column: {source}")

    selected: list[str] = []
    for line in lines[1:]:
        if not line.strip():
            continue
        row = next(csv.reader([line]))
        if row and row[0] == deck:
            selected.append(line)

    if not selected:
        raise ValueError(f"Deck {deck!r} is absent from {source}")

    # Preserve the simulator's original CSV bytes for the selected rows instead of
    # re-running that deck's fixed-seed population: https://github.com/FlareZ123/pokemon-sims/issues/2724
    # The paired aggregate is the repository's registered two-deck source:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
    with exclusive_lock(Path(f"{output}.lock")):
        atomic_write_text(output, lines[0] + "".join(selected))


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Project one deck's rows from an existing paired simulator matrix."
    )
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--deck", required=True)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    project_deck_matrix(args.source.resolve(), args.out.resolve(), args.deck)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
