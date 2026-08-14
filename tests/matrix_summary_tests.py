from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
SCRIPT = REPO_ROOT / "scripts" / "summarize_matrix.py"


def run_summary(csv_text: str) -> tuple[subprocess.CompletedProcess[str], str]:
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = Path(temporary_directory)
        source = root / "paired.csv"
        destination = root / "summary.md"
        source.write_text(csv_text, encoding="utf-8")
        completed = subprocess.run(
            [
                sys.executable,
                str(SCRIPT),
                "--input",
                str(source),
                "--output",
                str(destination),
            ],
            cwd=REPO_ROOT,
            check=False,
            capture_output=True,
            text=True,
        )
        summary = destination.read_text(encoding="utf-8") if destination.exists() else ""
        return completed, summary


def main() -> int:
    # Deliberately shuffle the CSV columns so the readable report is proven to use
    # names instead of positional/manual reconstruction:
    # https://github.com/FlareZ123/pokemon-sims/issues/3764
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/extract_deck_matrix.py
    completed, summary = run_summary(
        "scenario,ready_by_t3_pct,deck,ready_by_t2_pct,unused\n"
        "strict-jit/go-first,41.731,regidrago-shell,12.192,x\n"
        "strict-jit-turn2-item-lock/go-second,12.788,regidrago-pineco,6.790,y\n"
    )
    if completed.returncode != 0:
        raise AssertionError(completed.stderr)

    expected = """# T2/T3 Setup Probability Summary

Source: `paired.csv`

| Deck | Scenario | Ready by T2 | Ready by T3 |
| --- | --- | ---: | ---: |
| regidrago-shell | strict-jit/go-first | 12.192% | 41.731% |
| regidrago-pineco | strict-jit-turn2-item-lock/go-second | 6.790% | 12.788% |
"""
    if summary != expected:
        raise AssertionError(f"unexpected matrix summary:\n{summary}")

    missing_column, _ = run_summary(
        "deck,scenario,ready_by_t2_pct\n"
        "regidrago-shell,strict-jit/go-first,12.192\n"
        "regidrago-pineco,strict-jit/go-first,19.559\n"
    )
    if missing_column.returncode == 0:
        raise AssertionError("summary generation accepted a matrix without ready_by_t3_pct")
    if "matrix is missing required columns: ready_by_t3_pct" not in missing_column.stderr:
        raise AssertionError(f"unexpected missing-column failure: {missing_column.stderr}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
