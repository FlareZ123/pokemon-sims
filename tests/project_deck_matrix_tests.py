from __future__ import annotations

import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.project_deck_matrix import project_deck_matrix

WORKFLOW = REPO_ROOT / ".github" / "workflows" / "ci.yml"


def test_projection_preserves_selected_rows_exactly() -> None:
    source_text = (
        "deck,scenario,trials\n"
        '"regidrago-shell","strict-jit/go-first",100000\n'
        '"regidrago-shell","strict-jit/go-second",100000\n'
        '"regidrago-pineco","strict-jit/go-first",100000\n'
    )
    expected = (
        "deck,scenario,trials\n"
        '"regidrago-shell","strict-jit/go-first",100000\n'
        '"regidrago-shell","strict-jit/go-second",100000\n'
    )
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(source_text, encoding="utf-8", newline="\n")
        project_deck_matrix(source, output, "regidrago-shell")
        assert output.read_bytes() == expected.encode("utf-8")


def test_ci_runs_one_aggregate_population_and_projects_shell() -> None:
    workflow = WORKFLOW.read_text(encoding="utf-8")
    fixed_seed_population = "--trials 100000 --seed 20260705"

    # The canonical shell matrix must reuse the paired experiment instead of
    # starting a second fixed-seed population: https://github.com/FlareZ123/pokemon-sims/issues/2724
    assert workflow.count(fixed_seed_population) == 1
    assert "--all-decks --trials 100000 --seed 20260705" in workflow
    assert "scripts/project_deck_matrix.py" in workflow
    assert "--deck regidrago-shell" in workflow


def main() -> int:
    test_projection_preserves_selected_rows_exactly()
    test_ci_runs_one_aggregate_population_and_projects_shell()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
