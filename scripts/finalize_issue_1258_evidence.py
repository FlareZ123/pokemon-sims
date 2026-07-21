from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OLD = "337b39c091da7f87ca95c98724e6d245af61a77a6826a1fff25ceb06d08fb8d9"
NEW = "6fca4b3ab1e33f350fcf214aca894547705a7e518d7764c1ac5761a3bb9cd97b"
FILES = (
    ROOT / "results" / "baseline_manifest.json",
    ROOT / "results" / "multi_deck_manifest.json",
    ROOT / "docs" / "MULTI_DECK_REPORT.md",
)
SCRIPT = Path(__file__).resolve()
WORKFLOW = ROOT / ".github" / "workflows" / "finalize-issue-1258.yml"


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def main() -> None:
    for path in FILES:
        text = path.read_text(encoding="utf-8")
        if text.count(OLD) != 1:
            raise RuntimeError(f"Expected one stale digest in {path}.")
        atomic_write(path, text.replace(OLD, NEW, 1))
    SCRIPT.unlink()
    WORKFLOW.unlink()


if __name__ == "__main__":
    main()
