# Debug audit, 2026-07-22

This temporary PR triggers the repository's read-only pull-request CI against current `main` for a source-bound debug review. The existing CI performs eleven deterministic `--simulate-this` traces, canonical and paired 100,000-trial matrices, the complete Release suite, and ASan/UBSan validation.

Repository CI contract: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

Repository specification: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md

Policy specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

Official Pokémon TCG procedure: https://www.pokemon.com/us/pokemon-tcg/rules
