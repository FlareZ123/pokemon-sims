from __future__ import annotations

import os
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.extract_deck_matrix import extract_deck_rows

WORKFLOW = REPO_ROOT / ".github" / "workflows" / "ci.yml"


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


def test_ci_runs_one_fixed_seed_aggregate() -> None:
    workflow = WORKFLOW.read_text(encoding="utf-8")
    population = "--trials 100000 --seed 20260705"

    # The paired aggregate is the single source for the canonical shell rows:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
    # https://github.com/FlareZ123/pokemon-sims/issues/2724
    assert workflow.count(population) == 1
    assert "--all-decks --trials 100000 --seed 20260705" in workflow
    assert "scripts/extract_deck_matrix.py" in workflow
    assert "--deck regidrago-shell" in workflow


def run_dde_2grass_experiment() -> None:
    # Run the expensive paired experiment exactly once in the Release job. The
    # experiment runner patches a temporary source copy, so neither the checkout nor
    # canonical TCG Live matrices are changed. DDE remains paper-Expanded-only:
    # https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    # https://github.com/FlareZ123/pokemon-sims/issues/2332
    if os.environ.get("GITHUB_ACTIONS") != "true":
        return
    if os.environ.get("GITHUB_JOB") != "release":
        return
    if not os.environ.get("GITHUB_HEAD_REF", "").startswith("experiment/dde-2grass-"):
        return
    # This test script runs once as a dedicated contract step and again inside the
    # complete CTest suite. Reuse the first paired population instead of launching
    # another 3.2 million simulations in the same job:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
    if (REPO_ROOT / "trace-dde-2grass-summary.txt").exists():
        return
    subprocess.run(
        [sys.executable, str(REPO_ROOT / "experiments" / "run_dde_2grass_experiment.py")],
        cwd=REPO_ROOT,
        check=True,
    )


def main() -> int:
    test_exact_deck_row_extraction()
    test_ci_runs_one_fixed_seed_aggregate()
    run_dde_2grass_experiment()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
