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
    assert workflow.count(population) == 1
    assert "--all-decks --trials 100000 --seed 20260705" in workflow
    assert "scripts/extract_deck_matrix.py" in workflow
    assert "--deck regidrago-shell" in workflow


def run_ev_vs_quick_ball_ci_experiment() -> None:
    if os.environ.get("GITHUB_ACTIONS") != "true":
        return

    source = REPO_ROOT / "experiments" / "ev_vs_quick_ball.cpp"
    binary = REPO_ROOT / "ev-vs-quick-ball-experiment"
    compiler = os.environ.get("CXX", "c++")
    subprocess.run(
        [compiler, "-std=c++20", "-O2", "-I", str(REPO_ROOT), str(source), "-o", str(binary)],
        cwd=REPO_ROOT,
        check=True,
    )
    subprocess.run([str(binary)], cwd=REPO_ROOT, check=True)

    summary = REPO_ROOT / "trace-ev-vs-quick-ball-summary.txt"
    aggregate = (REPO_ROOT / "ev-vs-quick-ball.csv").read_text(encoding="utf-8")
    paired = (REPO_ROOT / "ev-vs-quick-ball-paired.csv").read_text(encoding="utf-8")
    witnesses = (REPO_ROOT / "ev-vs-quick-ball-witnesses.csv").read_text(encoding="utf-8")
    summary.write_text(
        "PAIRED EARTHEN VESSEL VS QUICK BALL EXPERIMENT\n"
        "Both recipes intentionally contain 59 effective cards: the original second Earthen Vessel is nulled from both.\n"
        "Variant A = 3 Quick Ball / 1 Earthen Vessel. Variant B = 4 Quick Ball / 0 Earthen Vessel.\n"
        "Each paired game resets mt19937_64 to the same per-game seed for both recipes.\n\n"
        "AGGREGATE\n" + aggregate + "\nPAIRED\n" + paired + "\nWITNESSES\n" + witnesses,
        encoding="utf-8",
        newline="",
    )
    print(summary.read_text(encoding="utf-8"))


def main() -> int:
    test_exact_deck_row_extraction()
    test_ci_runs_one_fixed_seed_aggregate()
    run_ev_vs_quick_ball_ci_experiment()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
