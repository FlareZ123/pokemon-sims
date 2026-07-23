# Current-main debug audit, 2026-07-23

This workflow-only audit is bound to `main@d58621bf8814e063a33be2a5e57b0aab46345ea5` and changes no simulator logic.

The permanent repository CI provides the validation surface for this pull request:

- eleven independent `--simulate-this` traces across the shell and Pineco recipes;
- the canonical shell 100,000-trial T2/T3 matrix;
- the paired two-deck 100,000-trial matrix across all 16 registered scenarios;
- the complete Release test suite;
- the complete ASan/UBSan suite.

Repository and evidence contracts:

- Simulator scope and matrix commands: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md
- K0/K1, DCI/JIT, earliest-ready, and resource priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
- Lock semantics and model boundaries: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
- Permanent CI definition: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml
- Official Pokémon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Regidrago VSTAR and Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

The static and trace review found one distinct unowned route-planning defect. Strict-JIT going-first seed 23 reaches T5 under current policy, while an observable K1 Star Alchemy to Grass, held Crispin, Quick Ball to Latias ex, and next-turn Professor Burnet route reaches legal setup readiness on T4. The report is filed separately and awaits two independent approvals before claim or implementation:

- https://github.com/FlareZ123/pokemon-sims/issues/1403

The existing open bug claims remain owned and active:

- https://github.com/FlareZ123/pokemon-sims/issues/1356
- https://github.com/FlareZ123/pokemon-sims/issues/1392
- https://github.com/FlareZ123/pokemon-sims/issues/1393
