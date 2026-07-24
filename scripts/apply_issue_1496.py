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
            raise SystemExit(f'{path}: expected one stale Crobat statement, found {count}')
        updated = text.replace(old, new, 1)
        temporary = path.with_suffix(path.suffix + '.issue1496.tmp')
        temporary.write_text(updated, encoding='utf-8')
        os.replace(temporary, path)


plan_path = Path('SIM-PLAN.md')
old_scope = (
    'Historical variant-builder work is retained here as design context. The current executable does not expose a variant builder or emit deck-swap rows; it writes baseline `all_scenarios()` rows only: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L271-L283. The prior `variant_results.csv` was removed and must not support current claims: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7.\n'
)
new_scope = (
    'Historical generic variant-builder work is retained here as design context. The current supported card-swap surface is the modeling-only Crobat V registry exposed through `--model-crobat` and `--model-variant`; these temporary derivatives remain outside `deck_registry()`, `--all-decks`, and the canonical shell baseline, and they write the source-bound `results/crobat_variant_model.csv` artifact and `docs/CROBAT_MODEL_REPORT.md`: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#L72-L86 https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://github.com/FlareZ123/pokemon-sims/issues/1394. The retired generic `variant_results.csv` screen remains withdrawn and cannot support current claims: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7 https://github.com/FlareZ123/pokemon-sims/issues/1496.\n'
)
atomic_replace(plan_path, old_scope, new_scope)

old_metric = (
    '- Baseline scenario probabilities. Matched-seed card-swap deltas remain a future extension until a current generator and result artifact are restored: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7.\n'
)
new_metric = (
    '- Source-bound paired Crobat V card-swap deltas for T2, T3, and T4 readiness, scenario improvements, Dark Asset use, and cards drawn per using game. The `--model-crobat` generator writes `results/crobat_variant_model.csv`, `--model-variant` reproduces one readable hand, and `docs/CROBAT_MODEL_REPORT.md` records the discrete cut costs and interpretation boundaries: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#L72-L86 https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1496.\n'
)
atomic_replace(plan_path, old_metric, new_metric)
