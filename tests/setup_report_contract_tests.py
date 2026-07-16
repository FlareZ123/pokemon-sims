from __future__ import annotations

import csv
import importlib.util
import json
import sys
from collections import Counter
from pathlib import Path
from types import ModuleType


REPO_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO_ROOT))
from scripts.baseline_provenance import simulator_policy_source_digest
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


def validate_scenario_inventory(rows: list[dict[str, str]], expected_scenarios: set[str]) -> None:
    # The aggregate matrix specification and canonical generator require exactly one
    # row for every scenario, rather than set membership alone:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/update_setup_docs.py#L14-L29
    scenario_counts = Counter(row["scenario"] for row in rows)
    observed_scenarios = set(scenario_counts)
    missing = sorted(expected_scenarios - observed_scenarios)
    extra = sorted(observed_scenarios - expected_scenarios)
    duplicates = sorted(scenario for scenario, count in scenario_counts.items() if count > 1)
    if missing or extra or duplicates or len(rows) != len(expected_scenarios):
        raise AssertionError(
            "Matrix scenario drift: "
            f"missing={missing}, extra={extra}, duplicates={duplicates}, "
            f"rows={len(rows)}, expected_rows={len(expected_scenarios)}."
        )


def assert_duplicate_scenario_is_rejected(
    rows: list[dict[str, str]], expected_scenarios: set[str]
) -> None:
    # Regression for duplicate-row provenance drift identified in issue #606:
    # https://github.com/FlareZ123/pokemon-sims/issues/606
    duplicate_rows = [*rows, dict(rows[0])]
    try:
        validate_scenario_inventory(duplicate_rows, expected_scenarios)
    except AssertionError as error:
        if "duplicates=" not in str(error):
            raise AssertionError("Duplicate scenario rejection lost its multiplicity diagnostic.") from error
    else:
        raise AssertionError("Duplicate scenario rows must fail the setup report provenance contract.")


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
    # The fixed-seed artifact must be regenerated whenever a simulation-policy input changes:
    # https://github.com/FlareZ123/pokemon-sims/issues/642
    # https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
    expected_source_digest = simulator_policy_source_digest(REPO_ROOT)
    recorded_source_digest = str(manifest.get("simulator_policy_source_sha256", ""))
    if recorded_source_digest != expected_source_digest:
        raise AssertionError(
            "Simulator policy changed after the published setup baseline. Regenerate "
            "results/simulation_results.csv and results/baseline_manifest.json before merging."
        )

    observed_trials = {int(row["trials"]) for row in rows}
    expected_trials = {int(manifest["trials"])}
    if observed_trials != expected_trials:
        raise AssertionError(
            f"Matrix trials {sorted(observed_trials)} do not match manifest {sorted(expected_trials)}."
        )

    expected_scenarios = set(generator.SCENARIO_LABELS)
    validate_scenario_inventory(rows, expected_scenarios)
    assert_duplicate_scenario_is_rejected(rows, expected_scenarios)

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
