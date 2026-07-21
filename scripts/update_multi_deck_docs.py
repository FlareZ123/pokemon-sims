from __future__ import annotations

import argparse
import csv
import json
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path

SCENARIO_LABELS = {
    "strict-jit/go-first": "Strict JIT, going first",
    "matchup-flex-jit/go-first": "Matchup-flex JIT, going first",
    "no-discard-control/go-first": "No discard control, going first",
    "strict-jit-turn2-item-lock/go-first": "Strict JIT, turn-two Item lock, first",
    "strict-jit-full-item-lock/go-first": "Strict JIT, full Item lock, first",
    "strict-jit-rulebox-ability-lock/go-first": "Strict JIT, Rule Box Ability lock, first",
    "strict-jit-combined-lock/go-first": "Strict JIT, combined lock, first",
    "strict-jit/go-second": "Strict JIT, going second",
    "matchup-flex-jit/go-second": "Matchup-flex JIT, going second",
    "no-discard-control/go-second": "No discard control, going second",
    "strict-jit-turn2-item-lock/go-second": "Strict JIT, turn-two Item lock, second",
    "strict-jit-full-item-lock/go-second": "Strict JIT, full Item lock, second",
    "strict-jit-rulebox-ability-lock/go-second": "Strict JIT, Rule Box Ability lock, second",
    "strict-jit-combined-lock/go-second": "Strict JIT, combined lock, second",
    "strict-jit-supporter-lock/go-first": "Strict JIT, Supporter lock, first",
    "strict-jit-supporter-lock/go-second": "Strict JIT, Supporter lock, second",
}
DECK_LABELS = {
    "regidrago-shell": "Regidrago shell",
    "regidrago-pineco": "Regidrago-Pineco with Secret Box",
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


def fmt(row: dict[str, str], field: str, digits: int = 3) -> str:
    return f"{float(row[field]):.{digits}f}"


def matrix_table(rows: list[dict[str, str]]) -> str:
    lines = [
        "| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |",
        "|---|---:|---:|---:|---:|",
    ]
    order = {scenario: index for index, scenario in enumerate(SCENARIO_LABELS)}
    for row in sorted(rows, key=lambda item: order[item["scenario"]]):
        lines.append(
            f"| {SCENARIO_LABELS[row['scenario']]} | "
            f"{fmt(row, 'ready_by_t2_pct')}% ± {fmt(row, 'ready_by_t2_se_pp')} | "
            f"{fmt(row, 'ready_by_t3_pct')}% ± {fmt(row, 'ready_by_t3_se_pp')} | "
            f"{fmt(row, 'ready_by_t4_pct')}% ± {fmt(row, 'ready_by_t4_se_pp')} | "
            f"{fmt(row, 'setup_failure_pct')}% ± {fmt(row, 'setup_failure_se_pp')} |"
        )
    return "\n".join(lines)


def first_ready_table(rows: list[dict[str, str]]) -> str:
    lines = [
        "| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |",
        "|---|---:|---:|---:|---:|",
    ]
    main = {
        "strict-jit/go-first",
        "strict-jit/go-second",
        "matchup-flex-jit/go-first",
        "matchup-flex-jit/go-second",
        "no-discard-control/go-first",
        "no-discard-control/go-second",
    }
    order = {scenario: index for index, scenario in enumerate(SCENARIO_LABELS)}
    for row in sorted(
        (r for r in rows if r["scenario"] in main),
        key=lambda item: order[item["scenario"]],
    ):
        lines.append(
            f"| {SCENARIO_LABELS[row['scenario']]} | {fmt(row, 'ready_on_t2_pct')}% | "
            f"{fmt(row, 'ready_on_t3_pct')}% | {fmt(row, 'ready_on_t4_pct')}% | "
            f"{fmt(row, 'ready_on_t5_pct')}% |"
        )
    return "\n".join(lines)


def comparison_table(by_deck: dict[str, list[dict[str, str]]]) -> str:
    scenarios = (
        "strict-jit/go-first",
        "strict-jit/go-second",
        "matchup-flex-jit/go-first",
        "matchup-flex-jit/go-second",
        "no-discard-control/go-first",
        "no-discard-control/go-second",
    )
    lookup = {
        (deck, row["scenario"]): row
        for deck, rows in by_deck.items()
        for row in rows
    }
    lines = [
        "| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for scenario in scenarios:
        shell = lookup[("regidrago-shell", scenario)]
        pineco = lookup[("regidrago-pineco", scenario)]
        values: list[str] = []
        for turn in (2, 3, 4):
            field = f"ready_by_t{turn}_pct"
            shell_value = float(shell[field])
            pineco_value = float(pineco[field])
            values.extend(
                (
                    f"{shell_value:.3f}%",
                    f"{pineco_value:.3f}%",
                    f"{pineco_value-shell_value:+.3f} pp",
                )
            )
        lines.append(f"| {SCENARIO_LABELS[scenario]} | " + " | ".join(values) + " |")
    return "\n".join(lines)


def diagnostics_table(row: dict[str, str]) -> str:
    fields = (
        ("Secret Box use", "secret_box_used_pct", "%"),
        ("Exploding Energy use", "exploding_energy_used_pct", "%"),
        ("Steven use", "steven_used_pct", "%"),
        ("Star Alchemy use", "star_alchemy_used_pct", "%"),
        ("Secret Box attempts", "secret_box_attempts_per_game", " per game"),
        ("Cost blocks", "secret_box_cost_blocks_per_game", " per game"),
        ("Missing route axis", "secret_box_missing_axis_per_game", " per game"),
        ("Bench blocks", "secret_box_bench_blocks_per_game", " per game"),
        ("Arven banks", "secret_box_arven_banks_per_game", " per game"),
        ("Steven banks", "secret_box_steven_banks_per_game", " per game"),
        ("Gladion banks", "secret_box_gladion_banks_per_game", " per game"),
        ("FSS banks", "secret_box_fss_banks_per_game", " per game"),
    )
    lines = ["| Route metric | Value |", "|---|---:|"]
    for label, field, suffix in fields:
        lines.append(f"| {label} | {fmt(row, field)}{suffix} |")
    return "\n".join(lines)


def axis_table(row: dict[str, str]) -> str:
    fields = (
        ("Regidrago line", "secret_box_missing_regi_axis_per_game"),
        ("Pineco/Forretress line", "secret_box_missing_line_axis_per_game"),
        ("VSTAR", "secret_box_missing_vstar_axis_per_game"),
        ("Payload", "secret_box_missing_payload_axis_per_game"),
        ("Search Item", "secret_box_missing_treasure_axis_per_game"),
        ("Fire", "secret_box_missing_fire_axis_per_game"),
        ("Grass", "secret_box_missing_grass_axis_per_game"),
        ("Ability", "secret_box_missing_ability_axis_per_game"),
        ("Supporter", "secret_box_missing_supporter_axis_per_game"),
        ("Known Prize zone", "secret_box_missing_known_prize_zone_per_game"),
        ("Discard zone", "secret_box_missing_discard_zone_per_game"),
        ("Stranded hand zone", "secret_box_missing_stranded_hand_zone_per_game"),
    )
    lines = ["| Overlapping failure reason | Events per game |", "|---|---:|"]
    for label, field in fields:
        lines.append(f"| {label} | {fmt(row, field)} |")
    return "\n".join(lines)


def report_markdown(rows: list[dict[str, str]], manifest: dict[str, object]) -> str:
    by_deck: dict[str, list[dict[str, str]]] = {
        deck: [] for deck in manifest["decks"]
    }
    for row in rows:
        by_deck[row["deck"]].append(row)
    pineco_no_control = next(
        row
        for row in by_deck["regidrago-pineco"]
        if row["scenario"] == "no-discard-control/go-second"
    )
    sections = []
    for deck in manifest["decks"]:
        sections.append(
            f"## {DECK_LABELS[deck]}\n\n{matrix_table(by_deck[deck])}\n\n"
            f"### First-ready-turn distribution\n\n{first_ready_table(by_deck[deck])}"
        )
    deck_sections = "\n\n".join(sections)
    return f"""# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `{manifest['matrix_seed']}`. Trials per condition: `{int(manifest['trials_per_condition']):,}`. Conditions: `{manifest['condition_count']}`. Total simulated games: `{int(manifest['total_simulated_games']):,}`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

{comparison_table(by_deck)}

{deck_sections}

## Secret Box route graph

```mermaid
graph LR
  MT[Mysterious Treasure] --> Tapu[Tapu Lele-GX]
  QB[Quick Ball] --> Tapu
  Tapu --> Arven
  Tapu --> Gladion
  Arven --> Box[Secret Box]
  Gladion -->|known prized| Box
  Steven[Steven's Resolve] --> Box
  FSS[Forest Seal Stone] --> Box
  Box --> Item[Mysterious Treasure or replacement Item]
  Box --> Tool[Forest Seal Stone when Fire is missing]
  Box --> Dawn
  Box --> Forest[Forest of Vitality when immediate evolution is needed]
  Dawn --> Pineco
  Dawn --> Forretress[Forretress ex]
  Dawn --> Payload[Dragon payload]
  Item --> VSTAR[Regidrago VSTAR]
  Tool --> Fire[Fire Energy]
  Forretress --> Grass[Grass Energy]
  Payload --> Discard[Strict-JIT discard]
  VSTAR --> Ready[Apex Dragon ready]
  Fire --> Ready
  Grass --> Ready
  Discard --> Ready
```

The graph is adaptive. Held cards, prior-turn setup, legal Prize knowledge, and ordinary evolution can remove a search category. The policy reserves every additional discard cost before paying Secret Box.

## Route-frequency diagnostics

The following row is `regidrago-pineco`, no-discard-control, going second. Counts may overlap because one rejected state can miss several axes.

{diagnostics_table(pineco_no_control)}

### Overlapping axis and zone counters

{axis_table(pineco_no_control)}

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `{manifest['simulator_policy_source_sha256']}`.

Comparison CSV SHA-256: `{manifest['comparison_csv_sha256']}`.
"""


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Update the named-deck comparison report."
    )
    parser.add_argument("--repo-root", type=Path, default=Path.cwd())
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = args.repo_root.resolve()
    manifest = json.loads(
        (repo_root / "results" / "multi_deck_manifest.json").read_text(
            encoding="utf-8"
        )
    )
    with (repo_root / "results" / "multi_deck_comparison.csv").open(
        newline="", encoding="utf-8"
    ) as handle:
        rows = list(csv.DictReader(handle))
    if not rows:
        raise ValueError("multi_deck_comparison.csv is empty")
    with exclusive_lock(repo_root / ".update-multi-deck-docs.lock"):
        atomic_write(
            repo_root / "docs" / "MULTI_DECK_REPORT.md",
            report_markdown(rows, manifest),
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
