# Open bug review audit, 2026-08-01

This branch exists only to trigger the repository's permanent pull-request CI against the current `main` source. It does not change simulator policy or production code.

The CI workflow performs more than three independent `--simulate-this` audits, complete Release tests, ASan/UBSan tests, and fixed-seed canonical plus paired matrices:

https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

Issues reviewed in this audit:

- https://github.com/FlareZ123/pokemon-sims/issues/2152
- https://github.com/FlareZ123/pokemon-sims/issues/2153
- https://github.com/FlareZ123/pokemon-sims/issues/2154

Card and rule sources used for the policy review:

- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
- Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
- Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
- Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Observable-information and route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md

The audit PR must be closed without merging after its CI artifacts are inspected.
