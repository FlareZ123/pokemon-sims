from __future__ import annotations

import argparse
import csv
import os
from collections import Counter
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


# This inventory mirrors the source registry emitted by --model-crobat.
# Live registry: https://github.com/FlareZ123/pokemon-sims/blob/fix/2247-purge-go-first-full-item-lock/src/trace_engine_v2/part_016.inc
# Klara card data: https://api.pokemontcg.io/v2/cards/swsh6-145
# Confirmed stale-artifact/source-binding bug: https://github.com/FlareZ123/pokemon-sims/issues/2253
EXPECTED_VARIANTS = (
    "regidrago-shell",
    "crobat1-erika",
    "crobat1-channeler",
    "crobat1-team-yell",
    "crobat1-klara",
    "crobat1-turo",
    "crobat1-powerglass",
    "crobat1-heavy-ball",
    "crobat1-tapu-lele",
    "crobat2-erika-channeler",
    "crobat2-erika-team-yell",
    "crobat2-erika-tapu-lele",
    "crobat2-tapu-lele-both",
)

# These are the current registered aggregate scenarios after issue #2247 retires
# both full-turn-one Item-lock rows while retaining turn-two Item lock.
# Scenario source: https://github.com/FlareZ123/pokemon-sims/blob/fix/2247-purge-go-first-full-item-lock/src/trace_engine_v2/part_016.inc
# Scenario correction: https://github.com/FlareZ123/pokemon-sims/issues/2247
# Artifact source-binding bug: https://github.com/FlareZ123/pokemon-sims/issues/2253
EXPECTED_SCENARIOS = (
    "strict-jit/go-first",
    "matchup-flex-jit/go-first",
    "no-discard-control/go-first",
    "strict-jit-turn2-item-lock/go-first",
    "strict-jit-rulebox-ability-lock/go-first",
    "strict-jit-combined-lock/go-first",
    "strict-jit/go-second",
    "matchup-flex-jit/go-second",
    "no-discard-control/go-second",
    "strict-jit-turn2-item-lock/go-second",
    "strict-jit-rulebox-ability-lock/go-second",
    "strict-jit-combined-lock/go-second",
    "strict-jit-supporter-lock/go-first",
    "strict-jit-supporter-lock/go-second",
)


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def validate_source_bound_inventory(rows: list[dict[str, str]]) -> None:
    # A complete stale matrix used to pass because only per-variant row counts were
    # checked. Validate the exact live variant/scenario Cartesian product instead:
    # https://github.com/FlareZ123/pokemon-sims/issues/2253
    pairs = Counter((row["variant"], row["scenario"]) for row in rows)
    duplicates = sorted(pair for pair, count in pairs.items() if count != 1)
    if duplicates:
        raise RuntimeError(f"Crobat modeling CSV has duplicate variant/scenario pairs: {duplicates}")

    actual_variants = {row["variant"] for row in rows}
    expected_variants = set(EXPECTED_VARIANTS)
    if actual_variants != expected_variants:
        missing = sorted(expected_variants - actual_variants)
        unexpected = sorted(actual_variants - expected_variants)
        raise RuntimeError(
            f"Crobat modeling variants do not match source registry; "
            f"missing={missing}, unexpected={unexpected}"
        )

    actual_scenarios = {row["scenario"] for row in rows}
    expected_scenarios = set(EXPECTED_SCENARIOS)
    if actual_scenarios != expected_scenarios:
        missing = sorted(expected_scenarios - actual_scenarios)
        unexpected = sorted(actual_scenarios - expected_scenarios)
        raise RuntimeError(
            f"Crobat modeling scenarios do not match registered aggregate inventory; "
            f"missing={missing}, unexpected={unexpected}"
        )

    expected_pairs = {
        (variant, scenario)
        for variant in EXPECTED_VARIANTS
        for scenario in EXPECTED_SCENARIOS
    }
    actual_pairs = set(pairs)
    if actual_pairs != expected_pairs:
        missing = sorted(expected_pairs - actual_pairs)
        unexpected = sorted(actual_pairs - expected_pairs)
        raise RuntimeError(
            f"Crobat modeling CSV is not the exact source-bound Cartesian product; "
            f"missing={missing}, unexpected={unexpected}"
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--csv", type=Path, default=Path("results/crobat_variant_model.csv"))
    parser.add_argument("--out", type=Path, default=Path("docs/CROBAT_MODEL_REPORT.md"))
    args = parser.parse_args()

    with args.csv.open(newline="", encoding="utf-8") as handle:
        rows = list(csv.DictReader(handle))
    if not rows:
        raise RuntimeError("Crobat modeling CSV is empty")

    validate_source_bound_inventory(rows)

    by_variant: dict[str, list[dict[str, str]]] = {}
    for row in rows:
        by_variant.setdefault(row["variant"], []).append(row)
    baseline = {row["scenario"]: row for row in by_variant["regidrago-shell"]}
    scenario_count = len(baseline)

    summaries: list[dict[str, object]] = []
    for variant, variant_rows in by_variant.items():
        if len(variant_rows) != scenario_count:
            raise RuntimeError(
                f"{variant} has {len(variant_rows)} rows; expected {scenario_count} registered scenarios"
            )
        deltas_t2 = []
        deltas_t3 = []
        deltas_t4 = []
        improvements = 0
        for row in variant_rows:
            base = baseline[row["scenario"]]
            d2 = float(row["ready_by_t2_pct"]) - float(base["ready_by_t2_pct"])
            d3 = float(row["ready_by_t3_pct"]) - float(base["ready_by_t3_pct"])
            d4 = float(row["ready_by_t4_pct"]) - float(base["ready_by_t4_pct"])
            deltas_t2.append(d2)
            deltas_t3.append(d3)
            deltas_t4.append(d4)
            improvements += d3 > 0
        use_total = sum(float(row["dark_asset_use_pct"]) for row in variant_rows)
        weighted_draw = sum(
            float(row["dark_asset_use_pct"]) * float(row["avg_dark_asset_cards_drawn_per_game_using"])
            for row in variant_rows
        )
        summaries.append(
            {
                "variant": variant,
                "cuts": variant_rows[0]["cuts"],
                "copies": variant_rows[0]["crobat_copies"],
                "t2": sum(deltas_t2) / len(deltas_t2),
                "t3": sum(deltas_t3) / len(deltas_t3),
                "t4": sum(deltas_t4) / len(deltas_t4),
                "improvements": improvements,
                "use": use_total / len(variant_rows),
                "draw": weighted_draw / use_total if use_total else 0.0,
            }
        )
    summaries.sort(key=lambda row: (float(row["t3"]), float(row["t2"])), reverse=True)
    best_crobat = max(
        (row for row in summaries if row["variant"] != "regidrago-shell"),
        key=lambda row: (float(row["t3"]), float(row["t2"])),
    )

    lines = [
        "# Crobat V modeling report",
        "",
        "## Scope",
        "",
        "This report compares temporary Crobat V swaps derived from `regidrago-shell`. "
        "The variants are generated by `--model-crobat` and remain outside `deck_registry()`, "
        "`--all-decks`, the canonical shell baseline, and the registered two-deck comparison.",
        "",
        "The checked model is source-bound to exactly 13 current variants and 14 registered aggregate "
        "scenarios, for 182 conditions. At the canonical 100,000 trials per condition, that is 18.2 "
        "million simulated games. The exact variant/scenario inventory is validated before report "
        "generation so a stale but complete matrix cannot silently pass: "
        "https://github.com/FlareZ123/pokemon-sims/issues/2253",
        "",
        "The one-Crobat recovery cut is Klara `swsh6-145`, matching the registered shell source. "
        "The retired `crobat1-roseanne` variant is rejected by the source-bound inventory check: "
        "https://api.pokemontcg.io/v2/cards/swsh6-145 "
        "https://github.com/FlareZ123/pokemon-sims/issues/1773 "
        "https://github.com/FlareZ123/pokemon-sims/issues/2253",
        "",
        "Turn-one full Item-lock rows are intentionally excluded from this model because "
        "`--model-crobat` follows the current registered aggregate scenarios. Combined lock uses "
        "Rule Box Ability suppression plus Item lock beginning on turn 2. These retired rows must "
        "not be restored as current-paper Expanded conditions: "
        "https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf "
        "https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ "
        "https://github.com/FlareZ123/pokemon-sims/issues/2247",
        "",
        "Crobat V `swsh3-104` uses Dark Asset only when played from hand to the Bench during a turn, "
        "draws until the hand contains six cards, and is limited to one use each turn: "
        "https://api.pokemontcg.io/v2/cards/swsh3-104",
        "",
        "## Result",
        "",
    ]
    if float(best_crobat["t3"]) > 0:
        lines.append(
            f"The fastest Crobat variant was `{best_crobat['variant']}`, averaging "
            f"{float(best_crobat['t3']):+.3f} percentage points of T3 readiness across all scenarios."
        )
    else:
        lines.append(
            f"No tested Crobat swap made the shell faster. The least damaging variant was "
            f"`{best_crobat['variant']}`, which averaged {float(best_crobat['t3']):+.3f} percentage "
            "points of T3 readiness across all scenarios."
        )
    lines.extend(
        [
            "",
            "Dark Asset is a probabilistic hand-refresh connector with a Bench cost. The tested cuts "
            "remove cards with recovery, lock-answer, gust, Tool, Prize-information, or matchup value. "
            "Those discrete losses remain relevant even when a setup percentage is close.",
            "",
            "Dark Asset utilization is the percentage of games with at least one legal use. "
            "The draw figure is total cards drawn across every Dark Asset resolution divided by "
            "games with at least one use, so two-copy variants may include uses on different turns: "
            "https://github.com/FlareZ123/pokemon-sims/issues/1394",
            "",
            "## Paired all-scenario summary",
            "",
            "| Variant | Cuts | Crobat | Mean ΔT2 | Mean ΔT3 | Mean ΔT4 | T3 scenarios improved | Games using Dark Asset | Mean cards drawn per using game |",
            "|---|---|---:|---:|---:|---:|---:|---:|---:|",
        ]
    )
    for row in summaries:
        lines.append(
            f"| `{row['variant']}` | {row['cuts']} | {row['copies']} | "
            f"{float(row['t2']):+.3f} pp | {float(row['t3']):+.3f} pp | "
            f"{float(row['t4']):+.3f} pp | {row['improvements']}/{scenario_count} | "
            f"{float(row['use']):.3f}% | {float(row['draw']):.3f} |"
        )
    lines.extend(
        [
            "",
            "## Interpretation boundaries",
            "",
            "The matrix measures the repository's setup objective through T4. It does not price Crobat V's "
            "two-Prize liability, Darkness typing, attack, opponent interaction, or the full match value of "
            "cards removed by each swap. Rule Box Ability lock suppresses Dark Asset, and opening Crobat V "
            "does not trigger it because setup is outside a turn: https://api.pokemontcg.io/v2/cards/swsh6-148 "
            "https://www.pokemon.com/us/pokemon-tcg/rules",
            "",
            "## Reproduction",
            "",
            "```bash",
            "./build/regidrago_sim --model-crobat --trials 100000 --seed 20260723 --out results/crobat_variant_model.csv",
            "python scripts/update_crobat_modeling_docs.py --csv results/crobat_variant_model.csv --out docs/CROBAT_MODEL_REPORT.md",
            "```",
            "",
            "Issue: https://github.com/FlareZ123/pokemon-sims/issues/1394",
            "Source-binding fix: https://github.com/FlareZ123/pokemon-sims/issues/2253",
            "",
        ]
    )

    with locked_file(args.out.with_suffix(args.out.suffix + ".lock")):
        atomic_write(args.out, "\n".join(lines))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
