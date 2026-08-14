from __future__ import annotations

import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.extract_deck_matrix import extract_deck_rows

WORKFLOW = REPO_ROOT / ".github" / "workflows" / "ci.yml"


MATRIX_HEADER = (
    "deck,scenario,trials,ready_by_t2_pct,ready_by_t3_pct,"
    "ready_by_t4_pct,ready_by_t5_pct\n"
)


def matrix_row(
    deck: str,
    scenario: str,
    t2: float,
    t3: float,
    t4: float,
    t5: float,
) -> str:
    return f'"{deck}","{scenario}",100000,{t2},{t3},{t4},{t5}\n'


# The regression preserves the canonical shell file as an exact row subset of
# the single paired aggregate instead of launching a second simulator experiment:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
# https://github.com/FlareZ123/pokemon-sims/issues/2724
def test_exact_deck_row_extraction() -> None:
    paired = (
        MATRIX_HEADER
        + matrix_row("regidrago-shell", "strict-jit/go-first", 12.19, 41.7, 59.1, 70.1)
        + matrix_row("regidrago-pineco", "strict-jit/go-first", 19.762, 49.0, 66.0, 75.0)
        + matrix_row("regidrago-shell", "strict-jit/go-second", 29.812, 55.5, 67.5, 75.5)
        + matrix_row("regidrago-pineco", "strict-jit/go-second", 48.305, 64.5, 72.0, 78.0)
    )
    expected = (
        MATRIX_HEADER
        + matrix_row("regidrago-shell", "strict-jit/go-first", 12.19, 41.7, 59.1, 70.1)
        + matrix_row("regidrago-shell", "strict-jit/go-second", 29.812, 55.5, 67.5, 75.5)
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


# A catastrophic aggregate must fail before extraction, even when it is otherwise
# well-formed and cumulative. This covers the 88-99% reporting class that exposed
# the missing sanity boundary:
# https://github.com/FlareZ123/pokemon-sims/issues/3761
def test_egregious_early_readiness_skew_is_rejected() -> None:
    paired = MATRIX_HEADER + matrix_row(
        "regidrago-shell", "strict-jit/go-first", 88.89, 99.524, 99.7, 99.8
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(paired, encoding="utf-8", newline="")

        try:
            extract_deck_rows(source, output, "regidrago-shell")
        except ValueError as error:
            message = str(error)
            assert "egregious early-readiness skew" in message
            assert "regidrago-shell/strict-jit/go-first" in message
            assert "ready_by_t2_pct" in message
        else:
            raise AssertionError("egregious T2/T3 statistical skew must fail")


def test_non_monotone_readiness_is_rejected() -> None:
    paired = MATRIX_HEADER + matrix_row(
        "regidrago-shell", "strict-jit/go-first", 20.0, 19.0, 50.0, 60.0
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(paired, encoding="utf-8", newline="")

        try:
            extract_deck_rows(source, output, "regidrago-shell")
        except ValueError as error:
            assert "cumulative readiness is not monotone" in str(error)
        else:
            raise AssertionError("non-monotone cumulative readiness must fail")


def test_out_of_range_readiness_is_rejected() -> None:
    paired = MATRIX_HEADER + matrix_row(
        "regidrago-shell", "strict-jit/go-first", -1.0, 40.0, 60.0, 70.0
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(paired, encoding="utf-8", newline="")

        try:
            extract_deck_rows(source, output, "regidrago-shell")
        except ValueError as error:
            assert "readiness percentage outside 0-100" in str(error)
        else:
            raise AssertionError("out-of-range cumulative readiness must fail")


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


def main() -> int:
    test_exact_deck_row_extraction()
    test_egregious_early_readiness_skew_is_rejected()
    test_non_monotone_readiness_is_rejected()
    test_out_of_range_readiness_is_rejected()
    test_ci_runs_one_fixed_seed_aggregate()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
