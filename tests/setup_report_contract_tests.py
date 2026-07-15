from __future__ import annotations

import csv
import importlib.util
import json
from pathlib import Path
from types import ModuleType


REPO_ROOT = Path(__file__).resolve().parents[1]
GENERATOR_PATH = REPO_ROOT / "scripts" / "update_setup_docs.py"
MANIFEST_PATH = REPO_ROOT / "results" / "baseline_manifest.json"
CSV_PATH = REPO_ROOT / "results" / "simulation_results.csv"
REPORT_PATH = REPO_ROOT / "docs" / "REPORT.md"


def load_generator() -> ModuleType:
    spec = importlib.util.spec_from_file_location("update_setup_docs", GENERATOR_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/update_setup_docs.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    generator = load_generator()
    manifest = json.loads(MANIFEST_PATH.read_text(encoding="utf-8"))
    with CSV_PATH.open(newline="", encoding="utf-8") as handle:
        reader = csv.DictReader(handle)
        rows = list(reader)
        fieldnames = list(reader.fieldnames or [])
    if not rows or not fieldnames:
        raise AssertionError("results/simulation_results.csv must contain the generated matrix.")

    # The documented aggregate command and manifest define one trial count for every row:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
    # https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
    observed_trials = {int(row["trials"]) for row in rows}
    expected_trials = {int(manifest["trials"])}
    if observed_trials != expected_trials:
        raise AssertionError(
            f"Matrix trials {sorted(observed_trials)} do not match manifest {sorted(expected_trials)}."
        )

    expected_scenarios = set(generator.SCENARIO_LABELS)
    observed_scenarios = {row["scenario"] for row in rows}
    if observed_scenarios != expected_scenarios:
        missing = sorted(expected_scenarios - observed_scenarios)
        extra = sorted(observed_scenarios - expected_scenarios)
        raise AssertionError(f"Matrix scenario drift: missing={missing}, extra={extra}.")

    # REPORT.md is generated from the CSV and manifest by this canonical function:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/update_setup_docs.py#L71-L120
    expected_report = generator.report_markdown(rows, fieldnames, manifest)
    actual_report = REPORT_PATH.read_text(encoding="utf-8")
    if actual_report != expected_report:
        raise AssertionError(
            "docs/REPORT.md is stale. Run "
            "`python scripts/update_setup_docs.py --repo-root .` after regenerating the matrix."
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
