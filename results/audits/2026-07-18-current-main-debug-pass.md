# Current-main debug validation

Source reviewed: `d8d5588dae44191931aeb644c10d94c2df108b7c`.

This validation-only change triggers the repository CI contract for a fresh current-main audit. It does not implement issue #917 or modify simulator policy.

Required evidence:

- three independent readable traces: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L32-L40
- complete Release and sanitizer suites: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L41-L79
- canonical T2/T3 matrix: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#what-the-probability-means
- policy objective and action priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
- readable trace interface: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
