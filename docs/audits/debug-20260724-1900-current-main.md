# Current-main debug audit, 2026-07-24 19:00 ET

Audit-only branch from `main` after PR #1524 merged. This marker triggers the existing pull-request CI without changing simulator policy.

Required evidence:

- at least three independent `--simulate-this` traces;
- canonical and paired 100,000-trial T2/T3 matrices;
- complete Release and ASan/UBSan suites.

Repository specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
