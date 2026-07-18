from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from contextlib import contextmanager
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.baseline_provenance import simulator_policy_source_digest

TRACE_SPECS = (
    ("strict-jit/go-second", 3, "strict_jit_go_second"),
    ("strict-jit/go-first", 4, "strict_jit_go_first"),
    ("matchup-flex-jit/go-second", 2, "matchup_flex_go_second"),
    ("strict-jit-turn2-item-lock/go-second", 3, "strict_turn2_item_lock_go_second"),
    ("strict-jit-rulebox-ability-lock/go-second", 3, "strict_rulebox_lock_go_second"),
    ("no-discard-control/go-first", 4, "no_discard_control_go_first"),
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


def find_trace_seed(executable: Path, scenario: str, deadline: int, max_seed: int) -> tuple[int, str]:
    for seed in range(1, max_seed + 1):
        completed = run(
            [
                str(executable),
                "--simulate-this",
                "--scenario",
                scenario,
                "--seed",
                str(seed),
                "--require-ready-by",
                str(deadline),
            ],
            check=False,
        )
        if completed.returncode == 0:
            return seed, completed.stdout
    raise RuntimeError(
        f"No passing seed found for {scenario} by turn {deadline} in 1..{max_seed}."
    )


def generate_matrix_atomic(executable: Path, matrix_path: Path, trials: int, matrix_seed: int) -> None:
    matrix_path.parent.mkdir(parents=True, exist_ok=True)
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{matrix_path.name}.", suffix=".tmp", dir=matrix_path.parent
    )
    os.close(descriptor)
    temporary_path = Path(temporary_name)
    try:
        # This is the repository's canonical fixed-seed aggregate command:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
        # https://github.com/FlareZ123/pokemon-sims/issues/642
        run(
            [
                str(executable),
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


def regenerate(executable: Path, output_dir: Path, max_seed: int, trials: int, matrix_seed: int) -> None:
    output_dir.mkdir(parents=True, exist_ok=True)
    trace_dir = output_dir / "traces"
    trace_dir.mkdir(parents=True, exist_ok=True)

    manifest: dict[str, object] = {
        "matrix_seed": matrix_seed,
        "trials": trials,
        # Bind the published matrix to every aggregate simulator input, including
        # part_016's scenario loop and seed derivation:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
        # https://github.com/FlareZ123/pokemon-sims/issues/642
        "simulator_policy_source_sha256": simulator_policy_source_digest(REPO_ROOT),
        "traces": [],
    }

    expected_trace_files: set[str] = set()
    for scenario, deadline, stem in TRACE_SPECS:
        seed, trace = find_trace_seed(executable, scenario, deadline, max_seed)
        file_name = f"{stem}_seed_{seed}.txt"
        expected_trace_files.add(file_name)
        atomic_write_text(trace_dir / file_name, trace)
        manifest["traces"].append(
            {
                "scenario": scenario,
                "deadline": deadline,
                "seed": seed,
                "file": file_name,
            }
        )

    generated_stems = "|".join(re.escape(stem) for _, _, stem in TRACE_SPECS)
    generated_trace_name = re.compile(rf"(?:{generated_stems})_seed_[0-9]+\.txt")
    for trace_path in trace_dir.iterdir():
        # Only canonical generator-owned trace names may be removed. This keeps
        # unrelated review notes while reconciling the directory to the new manifest:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md
        # https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
        # https://github.com/FlareZ123/pokemon-sims/issues/916
        if (trace_path.is_file() and generated_trace_name.fullmatch(trace_path.name) and
                trace_path.name not in expected_trace_files):
            trace_path.unlink()

    generate_matrix_atomic(executable, output_dir / "simulation_results.csv", trials, matrix_seed)
    atomic_write_text(
        output_dir / "baseline_manifest.json",
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Regenerate deterministic trace seeds and the aggregate setup matrix."
    )
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--out-dir", required=True, type=Path)
    parser.add_argument("--max-seed", type=int, default=5000)
    parser.add_argument("--trials", type=int, default=100000)
    parser.add_argument("--matrix-seed", type=int, default=20260705)
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    executable = args.exe.resolve()
    output_dir = args.out_dir.resolve()
    if not executable.is_file():
        raise FileNotFoundError(executable)
    with exclusive_lock(output_dir.parent / f".{output_dir.name}.lock"):
        regenerate(executable, output_dir, args.max_seed, args.trials, args.matrix_seed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
