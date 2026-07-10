from __future__ import annotations

import argparse
import csv
import json
import os
import re
import tempfile
from contextlib import contextmanager
from pathlib import Path

SCENARIO_LABELS = {
    "strict-jit/go-first": "Strict JIT, going first",
    "matchup-flex-jit/go-first": "Matchup-flex JIT, going first",
    "no-discard-control/go-first": "No discard control, going first",
    "strict-jit/go-second": "Strict JIT, going second",
    "matchup-flex-jit/go-second": "Matchup-flex JIT, going second",
    "no-discard-control/go-second": "No discard control, going second",
    "strict-jit-turn2-item-lock/go-first": "Strict JIT, turn-two Item lock, first",
    "strict-jit-full-item-lock/go-first": "Strict JIT, full Item lock, first",
    "strict-jit-rulebox-ability-lock/go-first": "Strict JIT, Rule Box Ability lock, first",
    "strict-jit-combined-lock/go-first": "Strict JIT, combined lock, first",
    "strict-jit-turn2-item-lock/go-second": "Strict JIT, turn-two Item lock, second",
    "strict-jit-full-item-lock/go-second": "Strict JIT, full Item lock, second",
    "strict-jit-rulebox-ability-lock/go-second": "Strict JIT, Rule Box Ability lock, second",
    "strict-jit-combined-lock/go-second": "Strict JIT, combined lock, second",
}


@contextmanager
def exclusive_lock(path: Path):
    descriptor = os.open(path, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        yield
    finally:
        os.close(descriptor)
        path.unlink(missing_ok=True)


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
    ) as handle:
        handle.write(text)
        temporary = Path(handle.name)
    os.replace(temporary, path)


def percent_column(fieldnames: list[str], turn: int) -> str:
    lowered = {name.lower(): name for name in fieldnames}
    exact_candidates = (
        f"ready_by_t{turn}_pct",
        f"ready_by_{turn}_pct",
        f"t{turn}_pct",
        f"ready_t{turn}_pct",
    )
    for candidate in exact_candidates:
        if candidate in lowered:
            return lowered[candidate]
    for name in fieldnames:
        compact = name.lower().replace("_", "")
        if f"t{turn}" in compact and ("pct" in compact or "percent" in compact):
            return name
    raise KeyError(f"No percentage column found for T{turn}: {fieldnames}")


def report_markdown(rows: list[dict[str, str]], fieldnames: list[str], manifest: dict[str, object]) -> str:
    scenario_column = "scenario" if "scenario" in fieldnames else fieldnames[0]
    t2_column = percent_column(fieldnames, 2)
    t3_column = percent_column(fieldnames, 3)
    t4_column = percent_column(fieldnames, 4)

    baseline = []
    locks = []
    for row in rows:
        scenario = row[scenario_column]
        target = locks if "lock" in scenario else baseline
        target.append(
            (
                SCENARIO_LABELS.get(scenario, scenario),
                row[t2_column],
                row[t3_column],
                row[t4_column],
            )
        )

    def table(entries: list[tuple[str, str, str, str]]) -> str:
        lines = ["| Scenario | T2 | T3 | T4 |", "|---|---:|---:|---:|"]
        for label, t2, t3, t4 in entries:
            lines.append(f"| {label} | {t2}% | {t3}% | {t4}% |")
        return "\n".join(lines)

    return f"""# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## {manifest['trials']:,}-trial baseline

Seed: `{manifest['matrix_seed']}`.

{table(baseline)}

## Lock stress tests

{table(locks)}

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
"""


def trace_audit_markdown(repo_root: Path, manifest: dict[str, object]) -> str:
    rows = []
    for entry in manifest["traces"]:
        trace_path = repo_root / "results" / "traces" / entry["file"]
        trace_text = trace_path.read_text(encoding="utf-8")
        ready_match = re.search(r"^T(\d+) \| READY \|", trace_text, flags=re.MULTILINE)
        if ready_match is None:
            raise ValueError(f"No READY trace line in {trace_path}")
        ready_turn = int(ready_match.group(1))
        command = (
            f"`{entry['scenario']}`, seed `{entry['seed']}` / "
            f"[`{entry['file']}`](../results/traces/{entry['file']})"
        )
        rows.append(
            f"| {SCENARIO_LABELS.get(entry['scenario'], entry['scenario'])} | {command} | T{ready_turn} | "
            "Exact executable trace is saved and the CTest deadline regression uses the same scenario, seed, and ready turn. |"
        )

    return """# Deterministic Trace Audit

## Method

The traces below are generated by the exact executable after the corrected setup sequence. Opening Active and optional Bench Pokémon are selected from the shuffled opening hand, then six Prize cards are placed from the remaining deck without another shuffle: https://www.pokemon.com/us/pokemon-tcg/rules.

Every state-changing trace line names its rule or policy IDs. Debug Prize output is never read before a legal deck or Prize inspection. The manifest records the scenario, deadline, seed, and saved filename for each regression.

## Reviewed traces

| Case | Command / saved trace | First ready | Validation |
|---|---|---:|---|
""" + "\n".join(rows) + """

## Reproduction

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
python scripts/regenerate_setup_baselines.py --exe build/regidrago_sim --out-dir regenerated-baselines
```

The generator uses an exclusive lock and atomic file replacement. The six saved traces and aggregate matrix are reproducible from [`../results/baseline_manifest.json`](../results/baseline_manifest.json).

## Boundary

This remains a setup-policy engine. It does not resolve a complete opposing board, damage, Knock Outs, Prize taking, hand disruption, gust, or all Expanded interactions.
"""


def update_readme(readme: str, manifest: dict[str, object]) -> str:
    strict_second = next(
        entry for entry in manifest["traces"] if entry["scenario"] == "strict-jit/go-second"
    )
    pattern = re.compile(
        r"(--scenario strict-jit/go-second --seed )\d+( --require-ready-by 3)"
    )
    updated, replacements = pattern.subn(
        rf"\g<1>{strict_second['seed']}\g<2>", readme, count=1
    )
    if replacements != 1:
        raise ValueError("README strict-jit/go-second sample command was not found exactly once")
    return updated


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Update generated setup report documents.")
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    manifest_path = repo_root / "results" / "baseline_manifest.json"
    csv_path = repo_root / "results" / "simulation_results.csv"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    with csv_path.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
        fieldnames = list(reader.fieldnames or [])
    if not rows or not fieldnames:
        raise ValueError("simulation_results.csv is empty")

    with exclusive_lock(repo_root / ".update-setup-docs.lock"):
        atomic_write(repo_root / "docs" / "REPORT.md", report_markdown(rows, fieldnames, manifest))
        atomic_write(repo_root / "docs" / "TRACE_AUDIT.md", trace_audit_markdown(repo_root, manifest))
        readme_path = repo_root / "README.md"
        atomic_write(readme_path, update_readme(readme_path.read_text(encoding="utf-8"), manifest))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
