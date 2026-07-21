from __future__ import annotations

import argparse
import hashlib
import json
import os
import subprocess
import sys
import tempfile
from contextlib import contextmanager
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.baseline_provenance import simulator_policy_source_digest

DECKS = ("regidrago-shell", "regidrago-pineco")
SCENARIOS = (
    "strict-jit/go-first",
    "matchup-flex-jit/go-first",
    "no-discard-control/go-first",
    "strict-jit-turn2-item-lock/go-first",
    "strict-jit-full-item-lock/go-first",
    "strict-jit-rulebox-ability-lock/go-first",
    "strict-jit-combined-lock/go-first",
    "strict-jit/go-second",
    "matchup-flex-jit/go-second",
    "no-discard-control/go-second",
    "strict-jit-turn2-item-lock/go-second",
    "strict-jit-full-item-lock/go-second",
    "strict-jit-rulebox-ability-lock/go-second",
    "strict-jit-combined-lock/go-second",
    "strict-jit-supporter-lock/go-first",
    "strict-jit-supporter-lock/go-second",
)

# Reviewed route contracts. Shell examples preserve representative historical
# scenarios. Pineco examples cover Steven assembly, Arven assembly with Active
# recovery, and known-Prize Gladion recovery:
# https://github.com/FlareZ123/pokemon-sims/issues/1118
TRACE_SPECS = (
    ("regidrago-shell", "strict-jit/go-second", 1, 3, "shell_strict_go_second_seed_1.txt"),
    ("regidrago-shell", "matchup-flex-jit/go-second", 3, 2, "shell_flex_go_second_seed_3.txt"),
    ("regidrago-shell", "no-discard-control/go-first", 1, 4, "shell_no_control_go_first_seed_1.txt"),
    ("regidrago-pineco", "strict-jit/go-second", 35, 2, "pineco_steven_secret_box_seed_35.txt"),
    ("regidrago-pineco", "strict-jit/go-second", 18, 2, "pineco_arven_active_recovery_seed_18.txt"),
    ("regidrago-pineco", "strict-jit/go-second", 40, 2, "pineco_gladion_prize_recovery_seed_40.txt"),
)


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


def run(command: list[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        command,
        check=check,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
    )


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def generate_matrix_atomic(
    executable: Path, matrix_path: Path, trials: int, matrix_seed: int
) -> None:
    matrix_path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{matrix_path.name}.", suffix=".tmp", dir=matrix_path.parent
    )
    os.close(descriptor)
    temporary_path = Path(temporary_name)
    try:
        run(
            [
                str(executable),
                "--all-decks",
                "--trials",
                str(trials),
                "--seed",
                str(matrix_seed),
                "--out",
                str(temporary_path),
            ]
        )
        os.replace(temporary_path, matrix_path)
    finally:
        temporary_path.unlink(missing_ok=True)


def generate_traces(executable: Path, trace_dir: Path) -> list[dict[str, object]]:
    trace_dir.mkdir(parents=True, exist_ok=True)
    manifest_entries: list[dict[str, object]] = []
    expected = {file_name for *_, file_name in TRACE_SPECS}
    for deck, scenario, seed, deadline, file_name in TRACE_SPECS:
        completed = run(
            [
                str(executable),
                "--simulate-this",
                "--deck",
                deck,
                "--scenario",
                scenario,
                "--seed",
                str(seed),
                "--require-ready-by",
                str(deadline),
            ]
        )
        atomic_write_text(trace_dir / file_name, completed.stdout)
        manifest_entries.append(
            {
                "deck": deck,
                "scenario": scenario,
                "seed": seed,
                "deadline": deadline,
                "file": file_name,
            }
        )
    for path in trace_dir.glob("*.txt"):
        if (
            path.name.startswith("shell_") or path.name.startswith("pineco_")
        ) and path.name not in expected:
            path.unlink()
    return manifest_entries


def generate(executable: Path, output_dir: Path, trials: int, matrix_seed: int) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    matrix_path = output_dir / "multi_deck_comparison.csv"
    generate_matrix_atomic(executable, matrix_path, trials, matrix_seed)
    traces = generate_traces(executable, output_dir / "multi_deck_traces")
    manifest = {
        "decks": list(DECKS),
        "scenarios": list(SCENARIOS),
        "matrix_seed": matrix_seed,
        "trials_per_condition": trials,
        "condition_count": len(DECKS) * len(SCENARIOS),
        "total_simulated_games": trials * len(DECKS) * len(SCENARIOS),
        "simulator_policy_source_sha256": simulator_policy_source_digest(REPO_ROOT),
        "comparison_csv_sha256": sha256(matrix_path),
        "traces": traces,
    }
    atomic_write_text(
        output_dir / "multi_deck_manifest.json",
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate the fixed-seed two-deck comparison and reviewed traces."
    )
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--out-dir", type=Path, default=REPO_ROOT / "results")
    parser.add_argument("--trials", type=int, default=100000)
    parser.add_argument("--matrix-seed", type=int, default=20260705)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    executable = args.exe.resolve()
    output_dir = args.out_dir.resolve()
    if not executable.is_file():
        raise FileNotFoundError(executable)
    if args.trials <= 0:
        raise ValueError("--trials must be positive")
    with exclusive_lock(output_dir.parent / f".{output_dir.name}-multi-deck.lock"):
        generate(executable, output_dir, args.trials, args.matrix_seed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
