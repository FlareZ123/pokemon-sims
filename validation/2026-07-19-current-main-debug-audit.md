# Current-main debug audit

Validation-only marker for `main@04744648e3d96b225faf7f54d1d7b46510240e5d`.

This pull request intentionally changes no simulator behavior, card data, policy, tests, generated baselines, or repository specifications. Its purpose is to trigger the repository CI workflow so the current main branch is evaluated through the eight independent `--simulate-this` audits, complete Release and sanitizer suites, and the canonical 100,000-trial T2/T3 matrix.

The pull request will be closed without merge after the CI evidence and traces are inspected.
