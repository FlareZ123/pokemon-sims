# Current-main debug audit, 2026-08-08

This behavior-neutral marker exists only to trigger a fresh pull-request CI audit from current `main` after #2370 merged.

The audit must run the repository's permanent `--simulate-this` trace checks and the canonical T2/T3 setup matrix. Trace review is judged against the repository policy contract and authoritative card/rule sources already cited beside the modeled actions:

- Policy contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
- Model assumptions: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
- Official Pokémon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules

This file changes no simulator behavior and is not intended for merge.
