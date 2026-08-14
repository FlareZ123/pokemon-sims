from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.extract_deck_matrix import extract_deck_rows

import matrix_summary_tests

WORKFLOW = REPO_ROOT / ".github" / "workflows" / "ci.yml"
EXTRACTOR = REPO_ROOT / "scripts" / "extract_deck_matrix.py"


# The regression preserves the canonical shell file as an exact row subset of
# the single paired aggregate instead of launching a second simulator experiment:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
# https://github.com/FlareZ123/pokemon-sims/issues/2724
def test_exact_deck_row_extraction() -> None:
    paired = (
        'deck,scenario,trials,ready_by_t2_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19\n'
        '"regidrago-pineco","strict-jit/go-first",100000,19.762\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812\n'
        '"regidrago-pineco","strict-jit/go-second",100000,48.305\n'
    )
    expected = (
        'deck,scenario,trials,ready_by_t2_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812\n'
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


def test_cli_emits_readable_summary_into_uploaded_tree() -> None:
    # The existing extraction step runs immediately after the single paired aggregate.
    # Its default summary path sits under build/Testing, which CI already uploads:
    # https://github.com/FlareZ123/pokemon-sims/issues/3764
    # https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml
    paired = (
        "deck,scenario,ready_by_t2_pct,ready_by_t3_pct\n"
        "regidrago-shell,strict-jit/go-first,12.192,41.731\n"
        "regidrago-pineco,strict-jit/go-first,19.559,48.998\n"
    )
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "multi-deck-matrix.csv"
        output = root / "t2-t3-matrix.csv"
        source.write_text(paired, encoding="utf-8")
        completed = subprocess.run(
            [
                sys.executable,
                str(EXTRACTOR),
                "--input",
                str(source),
                "--output",
                str(output),
                "--deck",
                "regidrago-shell",
            ],
            cwd=root,
            check=False,
            capture_output=True,
            text=True,
        )
        if completed.returncode != 0:
            raise AssertionError(completed.stderr)
        summary = root / "build" / "Testing" / "t2-t3-summary.md"
        assert summary.exists()
        summary_text = summary.read_text(encoding="utf-8")
        assert "12.192% | 41.731%" in summary_text
        assert "19.559% | 48.998%" in summary_text
        assert not Path(f"{summary}.lock").exists()


def test_ci_runs_one_fixed_seed_aggregate() -> None:
    workflow = WORKFLOW.read_text(encoding="utf-8")
    population = "--trials 100000 --seed 20260705"

    # The paired aggregate is the single source for the canonical shell rows, and
    # build/Testing is the already-uploaded evidence tree used by the summary:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
    # https://github.com/FlareZ123/pokemon-sims/issues/2724
    # https://github.com/FlareZ123/pokemon-sims/issues/3764
    assert workflow.count(population) == 1
    assert "--all-decks --trials 100000 --seed 20260705" in workflow
    assert "scripts/extract_deck_matrix.py" in workflow
    assert "--deck regidrago-shell" in workflow
    assert "build/Testing" in workflow


def main() -> int:
    test_exact_deck_row_extraction()
    test_cli_emits_readable_summary_into_uploaded_tree()
    test_ci_runs_one_fixed_seed_aggregate()
    matrix_summary_tests.main()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
