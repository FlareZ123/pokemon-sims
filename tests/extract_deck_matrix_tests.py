from __future__ import annotations

import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.extract_deck_matrix import (
    DEFAULT_SUMMARY_OUTPUT,
    extract_deck_rows,
    summarize_matrix,
)

WORKFLOW = REPO_ROOT / ".github" / "workflows" / "ci.yml"


def test_exact_deck_row_extraction() -> None:
    paired = (
        'deck,scenario,trials,ready_by_t2_pct,ready_by_t3_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19,41.731\n'
        '"regidrago-pineco","strict-jit/go-first",100000,19.762,49.028\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812,55.502\n'
        '"regidrago-pineco","strict-jit/go-second",100000,48.305,64.49\n'
    )
    expected = (
        'deck,scenario,trials,ready_by_t2_pct,ready_by_t3_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19,41.731\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812,55.502\n'
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(paired, encoding="utf-8", newline="")

        extract_deck_rows(source, output, "regidrago-shell")
        assert output.read_bytes() == expected.encode("utf-8")
        assert not Path(f"{output}.lock").exists()

        try:
            extract_deck_rows(source, output, "missing-deck")
        except ValueError as error:
            assert "deck not found" in str(error)
        else:
            raise AssertionError("missing deck must fail")


def test_canonical_t2_t3_summary() -> None:
    paired = (
        'deck,scenario,trials,ready_by_t3_pct,ready_by_t2_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,41.731,12.192\n'
        '"regidrago-pineco","strict-jit/go-first",100000,49.028,19.618\n'
    )
    expected = (
        "# Generated T2/T3 setup summary\n"
        "\n"
        "| Deck | Scenario | Ready by T2 | Ready by T3 |\n"
        "|---|---|---:|---:|\n"
        "| regidrago-shell | strict-jit/go-first | 12.192% | 41.731% |\n"
        "| regidrago-pineco | strict-jit/go-first | 19.618% | 49.028% |\n"
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / DEFAULT_SUMMARY_OUTPUT.name
        source.write_text(paired, encoding="utf-8", newline="")

        summarize_matrix(source, output)
        assert output.read_text(encoding="utf-8") == expected
        assert not Path(f"{output}.lock").exists()


def test_ci_runs_one_fixed_seed_aggregate() -> None:
    workflow = WORKFLOW.read_text(encoding="utf-8")
    population = "--trials 100000 --seed 20260705"

    assert workflow.count(population) == 1
    assert "--all-decks --trials 100000 --seed 20260705" in workflow
    assert "scripts/extract_deck_matrix.py" in workflow
    assert "--deck regidrago-shell" in workflow
    assert DEFAULT_SUMMARY_OUTPUT.name.startswith("trace-")
    assert "trace-*.txt" in workflow


def main() -> int:
    test_exact_deck_row_extraction()
    test_canonical_t2_t3_summary()
    test_ci_runs_one_fixed_seed_aggregate()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
