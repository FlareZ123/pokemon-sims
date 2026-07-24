# Debug audit 2026-07-24 run 3

This temporary branch exists only to trigger pull-request CI against current `main` for an independent simulator audit.

Audit requirements:

- inspect the full Release and sanitizer suites;
- review at least three independent `--simulate-this` traces for legal and resource-preserving play;
- inspect the generated canonical and paired T2/T3 setup matrices;
- do not merge this marker file.

Repository policy and trace specifications:

- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
- https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULES_TRACEABILITY.md
- https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml
