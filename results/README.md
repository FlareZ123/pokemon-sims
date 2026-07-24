# Reproducible results and trace artifacts

`simulation_results.csv` is the current 100,000-trial baseline produced with seed `20260705`. The manifest records a SHA-256 digest of every aggregate simulator source input, including the scenario loop and seed derivation in `part_016.inc`. The setup-report contract rejects simulator changes until the matrix and manifest are regenerated:

https://github.com/FlareZ123/pokemon-sims/issues/642
https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

The same source-bound run records T2 through T5 readiness, T5-only diagnostic recoveries, and setup failures under the repository's T4 success deadline.

`traces/` contains deterministic `--simulate-this` transcripts used for manual review. Each state-changing line includes the rule IDs documented in [`../docs/RULES_TRACEABILITY.md`](../docs/RULES_TRACEABILITY.md).

The retired generic `variant_results.csv` screen was produced before the trace-policy rewrite. It remains unsupported and must not be reused. The current modeling-only Crobat V registry is a separate surface: `--model-crobat` writes `results/crobat_variant_model.csv`, `--model-variant` reproduces one temporary derivative, and those recipes remain outside `deck_registry()` plus `--all-decks`: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/results/crobat_variant_model.csv https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1511

The C++ writer uses a destination lock and atomic replacement. The Python baseline generator also writes the aggregate matrix atomically. CTest covers the trace matrix, core fixtures, Tier Two policy fixtures, and the aggregate smoke run.
