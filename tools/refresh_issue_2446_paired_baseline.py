from __future__ import annotations

import csv
import fcntl
import hashlib
import json
import os
from pathlib import Path
import tempfile

from scripts.baseline_provenance import simulator_policy_source_digest

ROOT = Path(__file__).resolve().parents[1]
GENERATED = ROOT / "paired-new.csv"
COMPARISON = ROOT / "results/multi_deck_comparison.csv"
MANIFEST = ROOT / "results/multi_deck_manifest.json"
REPORT = ROOT / "docs/MULTI_DECK_REPORT.md"
LOCK = ROOT / "results/.issue_2446_refresh.lock"

SCENARIO_LABELS = {
    "strict-jit/go-first": "Strict JIT, going first",
    "matchup-flex-jit/go-first": "Matchup-flex JIT, going first",
    "no-discard-control/go-first": "No discard control, going first",
    "strict-jit-turn2-item-lock/go-first": "Strict JIT, turn-two Item lock, first",
    "strict-jit-rulebox-ability-lock/go-first": "Strict JIT, Rule Box Ability lock, first",
    "strict-jit-combined-lock/go-first": "Strict JIT, turn-two Item + Rule Box Ability lock, first",
    "strict-jit/go-second": "Strict JIT, going second",
    "matchup-flex-jit/go-second": "Matchup-flex JIT, going second",
    "no-discard-control/go-second": "No discard control, going second",
    "strict-jit-turn2-item-lock/go-second": "Strict JIT, turn-two Item lock, second",
    "strict-jit-rulebox-ability-lock/go-second": "Strict JIT, Rule Box Ability lock, second",
    "strict-jit-combined-lock/go-second": "Strict JIT, turn-two Item + Rule Box Ability lock, second",
    "strict-jit-supporter-lock/go-first": "Strict JIT, Supporter lock, first",
    "strict-jit-supporter-lock/go-second": "Strict JIT, Supporter lock, second",
}
LABEL_SCENARIOS = {label: scenario for scenario, label in SCENARIO_LABELS.items()}
DIRECT_LABELS = {
    "Strict JIT, going first",
    "Strict JIT, going second",
    "Matchup-flex JIT, going first",
    "Matchup-flex JIT, going second",
    "No discard control, going first",
    "No discard control, going second",
}
ROUTE_FIELDS = {
    "Secret Box use": ("secret_box_used_pct", "%"),
    "Exploding Energy use": ("exploding_energy_used_pct", "%"),
    "Steven use": ("steven_used_pct", "%"),
    "Star Alchemy use": ("star_alchemy_used_pct", "%"),
    "Secret Box attempts": ("secret_box_attempts_per_game", " per game"),
    "Cost blocks": ("secret_box_cost_blocks_per_game", " per game"),
    "Missing route axis": ("secret_box_missing_axis_per_game", " per game"),
    "Bench blocks": ("secret_box_bench_blocks_per_game", " per game"),
    "Arven banks": ("secret_box_arven_banks_per_game", " per game"),
    "Steven banks": ("secret_box_steven_banks_per_game", " per game"),
    "Gladion banks": ("secret_box_gladion_banks_per_game", " per game"),
    "FSS banks": ("secret_box_fss_banks_per_game", " per game"),
}
AXIS_FIELDS = {
    "Regidrago line": "secret_box_missing_regi_axis_per_game",
    "Pineco/Forretress line": "secret_box_missing_line_axis_per_game",
    "VSTAR": "secret_box_missing_vstar_axis_per_game",
    "Payload": "secret_box_missing_payload_axis_per_game",
    "Search Item": "secret_box_missing_treasure_axis_per_game",
    "Fire": "secret_box_missing_fire_axis_per_game",
    "Grass": "secret_box_missing_grass_axis_per_game",
    "Ability": "secret_box_missing_ability_axis_per_game",
    "Supporter": "secret_box_missing_supporter_axis_per_game",
    "Known Prize zone": "secret_box_missing_known_prize_zone_per_game",
    "Discard zone": "secret_box_missing_discard_zone_per_game",
    "Stranded hand zone": "secret_box_missing_stranded_hand_zone_per_game",
}


def atomic_write(path: Path, data: bytes) -> None:
    fd, temp_name = tempfile.mkstemp(prefix=path.name + ".", dir=path.parent)
    try:
        with os.fdopen(fd, "wb") as handle:
            handle.write(data)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temp_name, path)
    finally:
        if os.path.exists(temp_name):
            os.unlink(temp_name)


def fmt_pct(value: str) -> str:
    return f"{float(value):.3f}%"


def fmt_se(value: str) -> str:
    return f"{float(value):.3f}"


def scenario_row(rows: dict[tuple[str, str], dict[str, str]], deck: str, scenario: str) -> dict[str, str]:
    return rows[(deck, scenario)]


def rewrite_report(text: str, rows: dict[tuple[str, str], dict[str, str]], digest: str, comparison_hash: str) -> str:
    output: list[str] = []
    section2 = ""
    section3 = ""
    pineco_diag = scenario_row(rows, "regidrago-pineco", "no-discard-control/go-second")

    for line in text.splitlines():
        if line.startswith("## ") and not line.startswith("### "):
            section2 = line
            section3 = ""
        elif line.startswith("### "):
            section3 = line

        if line.startswith("| "):
            cells = [cell.strip() for cell in line.strip().strip("|").split("|")]
            label = cells[0] if cells else ""
            scenario = LABEL_SCENARIOS.get(label)

            if section2 == "## Direct comparison" and scenario and label in DIRECT_LABELS:
                shell = scenario_row(rows, "regidrago-shell", scenario)
                pineco = scenario_row(rows, "regidrago-pineco", scenario)
                shell_t2 = float(shell["ready_by_t2_pct"])
                pineco_t2 = float(pineco["ready_by_t2_pct"])
                shell_t3 = float(shell["ready_by_t3_pct"])
                pineco_t3 = float(pineco["ready_by_t3_pct"])
                shell_t4 = float(shell["ready_by_t4_pct"])
                pineco_t4 = float(pineco["ready_by_t4_pct"])
                line = (
                    f"| {label} | {shell_t2:.3f}% | {pineco_t2:.3f}% | {pineco_t2 - shell_t2:+.3f} pp | "
                    f"{shell_t3:.3f}% | {pineco_t3:.3f}% | {pineco_t3 - shell_t3:+.3f} pp | "
                    f"{shell_t4:.3f}% | {pineco_t4:.3f}% | {pineco_t4 - shell_t4:+.3f} pp |"
                )
            elif section2 == "## Regidrago shell" and section3 == "" and scenario:
                row = scenario_row(rows, "regidrago-shell", scenario)
                line = (
                    f"| {label} | {fmt_pct(row['ready_by_t2_pct'])} ± {fmt_se(row['ready_by_t2_se_pp'])} | "
                    f"{fmt_pct(row['ready_by_t3_pct'])} ± {fmt_se(row['ready_by_t3_se_pp'])} | "
                    f"{fmt_pct(row['ready_by_t4_pct'])} ± {fmt_se(row['ready_by_t4_se_pp'])} | "
                    f"{fmt_pct(row['setup_failure_pct'])} ± {fmt_se(row['setup_failure_se_pp'])} |"
                )
            elif section2 == "## Regidrago-Pineco with Secret Box" and section3 == "" and scenario:
                row = scenario_row(rows, "regidrago-pineco", scenario)
                line = (
                    f"| {label} | {fmt_pct(row['ready_by_t2_pct'])} ± {fmt_se(row['ready_by_t2_se_pp'])} | "
                    f"{fmt_pct(row['ready_by_t3_pct'])} ± {fmt_se(row['ready_by_t3_se_pp'])} | "
                    f"{fmt_pct(row['ready_by_t4_pct'])} ± {fmt_se(row['ready_by_t4_se_pp'])} | "
                    f"{fmt_pct(row['setup_failure_pct'])} ± {fmt_se(row['setup_failure_se_pp'])} |"
                )
            elif section3 == "### First-ready-turn distribution" and scenario:
                deck = "regidrago-shell" if section2 == "## Regidrago shell" else "regidrago-pineco"
                row = scenario_row(rows, deck, scenario)
                line = (
                    f"| {label} | {fmt_pct(row['ready_on_t2_pct'])} | {fmt_pct(row['ready_on_t3_pct'])} | "
                    f"{fmt_pct(row['ready_on_t4_pct'])} | {fmt_pct(row['ready_on_t5_pct'])} |"
                )
            elif section2 == "## Route-frequency diagnostics" and label in ROUTE_FIELDS:
                field, suffix = ROUTE_FIELDS[label]
                value = f"{float(pineco_diag[field]):.3f}{suffix}"
                line = f"| {label} | {value} |"
            elif section3 == "### Overlapping axis and zone counters" and label in AXIS_FIELDS:
                line = f"| {label} | {float(pineco_diag[AXIS_FIELDS[label]]):.3f} |"

        if section2 == "## Provenance" and line.startswith("Simulator policy digest:"):
            line = f"Simulator policy digest: `{digest}`."
        elif section2 == "## Provenance" and line.startswith("Comparison CSV SHA-256:"):
            line = f"Comparison CSV SHA-256: `{comparison_hash}`."

        output.append(line)

    return "\n".join(output) + "\n"


def main() -> None:
    if not GENERATED.is_file():
        raise SystemExit("paired-new.csv is missing")
    generated_bytes = GENERATED.read_bytes()
    digest = simulator_policy_source_digest(ROOT)
    comparison_hash = hashlib.sha256(generated_bytes).hexdigest()

    with GENERATED.open(newline="", encoding="utf-8") as handle:
        parsed = list(csv.DictReader(handle))
    if len(parsed) != 28:
        raise SystemExit(f"expected 28 paired rows, got {len(parsed)}")
    rows = {(row["deck"], row["scenario"]): row for row in parsed}
    if len(rows) != 28:
        raise SystemExit("paired matrix contains duplicate deck/scenario rows")

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    if manifest["simulator_policy_source_sha256"] != digest:
        raise SystemExit("source-bound provenance is stale before paired refresh")
    manifest["comparison_csv_sha256"] = comparison_hash

    report_text = REPORT.read_text(encoding="utf-8")
    refreshed_report = rewrite_report(report_text, rows, digest, comparison_hash)

    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with LOCK.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        atomic_write(COMPARISON, generated_bytes)
        atomic_write(MANIFEST, (json.dumps(manifest, indent=2) + "\n").encode("utf-8"))
        atomic_write(REPORT, refreshed_report.encode("utf-8"))
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    LOCK.unlink(missing_ok=True)

    print(f"source digest: {digest}")
    print(f"paired comparison sha256: {comparison_hash}")


if __name__ == "__main__":
    main()
