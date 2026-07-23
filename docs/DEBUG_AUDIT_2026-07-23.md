# Current-main debug audit, 2026-07-23

This audit-only pull request triggers the repository's permanent pull-request CI against the exact current `main` source. It does not change simulator logic, deck recipes, policy, rules, tests, or generated baselines.

The review covers the eight shell and three Pineco `--simulate-this` traces already specified in `.github/workflows/ci.yml`, with at least three traces manually checked for legal and route-optimal play. It also records the source-bound 100,000-trial T2/T3 shell matrix and paired two-deck matrix emitted by CI.

Permanent CI specification: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

Repository decision priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

Dynamic DCI model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation

Official Pokémon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
