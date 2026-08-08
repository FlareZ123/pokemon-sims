from __future__ import annotations

import csv
import hashlib
import importlib.util
import json
import sys
from collections import Counter
from pathlib import Path
from types import ModuleType

REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))

from scripts.baseline_provenance import simulator_policy_source_digest
from scripts.generate_multi_deck_comparison import DECKS, SCENARIOS, TRACE_SPECS

CSV_PATH = REPO_ROOT / "results" / "multi_deck_comparison.csv"
MANIFEST_PATH = REPO_ROOT / "results" / "multi_deck_manifest.json"
REPORT_PATH = REPO_ROOT / "docs" / "MULTI_DECK_REPORT.md"
SHELL_CSV_PATH = REPO_ROOT / "results" / "simulation_results.csv"
GENERATOR_PATH = REPO_ROOT / "scripts" / "update_multi_deck_docs.py"


def load_generator() -> ModuleType:
    spec = importlib.util.spec_from_file_location("update_multi_deck_docs", GENERATOR_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/update_multi_deck_docs.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    digest.update(path.read_bytes())
    return digest.hexdigest()


def main() -> int:
    generator = load_generator()
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    if not rows:
        raise AssertionError("The named-deck comparison CSV is empty.")

    if tuple(manifest["decks"]) != DECKS:
        raise AssertionError(f"Unexpected deck registry in manifest: {manifest['decks']}")
    if "regidrago-pineco-blender" in manifest["decks"]:
        raise AssertionError("The withdrawn Pineco Blender deck reappeared.")
    if tuple(manifest["scenarios"]) != SCENARIOS:
        raise AssertionError("The manifest scenario order drifted.")

    counts = Counter((row["deck"], row["scenario"]) for row in rows)
    expected = {(deck, scenario) for deck in DECKS for scenario in SCENARIOS}
    if set(counts) != expected or any(count != 1 for count in counts.values()):
        raise AssertionError("The comparison must contain one row per deck and scenario.")
    if int(manifest["condition_count"]) != len(expected):
        raise AssertionError("The manifest condition count is incorrect.")
    trials = int(manifest["trials_per_condition"])
    if {int(row["trials"]) for row in rows} != {trials}:
        raise AssertionError("Comparison row trials do not match the manifest.")
    if int(manifest["total_simulated_games"]) != trials * len(expected):
        raise AssertionError("The total simulated game count is incorrect.")

    if manifest["comparison_csv_sha256"] != sha256(CSV_PATH):
        raise AssertionError("The comparison CSV digest is stale.")
    if manifest["simulator_policy_source_sha256"] != simulator_policy_source_digest(REPO_ROOT):
        raise AssertionError("The named-deck comparison is stale for the current simulator.")

    with SHELL_CSV_PATH.open(newline="", encoding="utf-8") as handle:
        shell_rows = {row["scenario"]: row for row in csv.DictReader(handle)}
    comparison_shell = {
        row["scenario"]: row for row in rows if row["deck"] == "regidrago-shell"
    }
    if set(shell_rows) != set(comparison_shell):
        raise AssertionError("Canonical shell and paired comparison scenarios differ.")
    shared_fields = set(next(iter(shell_rows.values()))) & set(next(iter(comparison_shell.values())))
    shared_fields.discard("deck")
    for scenario, canonical in shell_rows.items():
        paired = comparison_shell[scenario]
        mismatches = [field for field in shared_fields if canonical[field] != paired[field]]
        if mismatches:
            raise AssertionError(
                f"Paired shell row drifted for {scenario}: {sorted(mismatches)}"
            )

    expected_trace_files = {file_name for *_, file_name in TRACE_SPECS}
    manifest_trace_files = {entry["file"] for entry in manifest["traces"]}
    if manifest_trace_files != expected_trace_files:
        raise AssertionError("Reviewed named-deck trace inventory drifted.")
    for entry in manifest["traces"]:
        trace_path = REPO_ROOT / "results" / "multi_deck_traces" / entry["file"]
        trace = trace_path.read_text(encoding="utf-8")
        if f"Deck: {entry['deck']}" not in trace or " | READY | " not in trace:
            raise AssertionError(f"Reviewed trace is incomplete: {trace_path}")

    expected_report = generator.report_markdown(rows, manifest)
    if REPORT_PATH.read_text(encoding="utf-8") != expected_report:
        raise AssertionError("docs/MULTI_DECK_REPORT.md is stale.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
