import fcntl
import os
from pathlib import Path


def atomic_replace(path: Path, old: str, new: str) -> None:
    lock_path = Path('/tmp') / f'{path.name}.issue1496.lock'
    with lock_path.open('w', encoding='utf-8') as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        text = path.read_text(encoding='utf-8')
        count = text.count(old)
        if count != 1:
            raise SystemExit(f'{path}: expected one matching contract, found {count}')
        updated = text.replace(old, new, 1)
        temporary = path.with_suffix(path.suffix + '.issue1496.tmp')
        temporary.write_text(updated, encoding='utf-8')
        os.replace(temporary, path)


plan_path = Path('SIM-PLAN.md')
old_scope = (
    'The executable registers `regidrago-shell` and `regidrago-pineco`. Aggregate `--all-decks` runs the same scenario matrix for both recipes and emits one row per recipe and scenario: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#registered-decks https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv https://github.com/FlareZ123/pokemon-sims/issues/1493. Historical generic variant-builder work remains design context; the retired `variant_results.csv` must not support current claims: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7.\n'
)
new_scope = (
    'The executable registers `regidrago-shell` and `regidrago-pineco`. Aggregate `--all-decks` runs the same scenario matrix for both recipes and emits one row per recipe and scenario: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#registered-decks https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv https://github.com/FlareZ123/pokemon-sims/issues/1493. The separate modeling-only Crobat V registry is exposed through `--model-crobat` and `--model-variant`; its temporary shell derivatives remain outside `deck_registry()`, `--all-decks`, and the canonical shell baseline, and write `results/crobat_variant_model.csv` plus `docs/CROBAT_MODEL_REPORT.md`: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1496. Historical generic variant-builder work remains design context; the retired generic `variant_results.csv` must not support current claims: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7.\n'
)
atomic_replace(plan_path, old_scope, new_scope)

old_metric = (
    '- Registered-deck scenario probabilities for `regidrago-shell` and `regidrago-pineco`, including the paired `--all-decks` artifact: https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv https://github.com/FlareZ123/pokemon-sims/issues/1493\n'
)
new_metric = old_metric + (
    '- Source-bound paired Crobat V card-swap deltas for T2, T3, and T4 readiness, scenario improvements, Dark Asset use, and cards drawn per using game. The `--model-crobat` generator writes `results/crobat_variant_model.csv`, `--model-variant` reproduces one readable hand, and `docs/CROBAT_MODEL_REPORT.md` records discrete cut costs and interpretation boundaries: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1496\n'
)
atomic_replace(plan_path, old_metric, new_metric)

old_validation = (
    '6. Smoke-test both registered recipes and byte-compare the aggregate `--all-decks` matrix. The retired generic `variant_results.csv` remains excluded: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L133-L140 https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7 https://github.com/FlareZ123/pokemon-sims/issues/1493\n'
)
new_validation = (
    '6. Smoke-test both registered recipes and byte-compare the aggregate `--all-decks` matrix. Separately regenerate the modeling-only Crobat V matrix, validate one `--model-variant` trace, and keep the retired generic `variant_results.csv` excluded: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L133-L140 https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1493 https://github.com/FlareZ123/pokemon-sims/issues/1496\n'
)
atomic_replace(plan_path, old_validation, new_validation)
