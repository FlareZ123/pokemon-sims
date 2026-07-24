# Source-bound debug audit, 2026-07-24

This temporary pull request exists only to run the repository's current-main CI against the checked-in simulator policy.

The audit requires review of at least three independent `--simulate-this` traces, the canonical 100,000-trial shell matrix, the paired 3.2-million-game matrix, the complete Release suite, and ASan/UBSan. The branch must not be merged. Close it after the artifacts and traces are reviewed and any distinct defects are filed separately.

Repository validation contract: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

Repository policy specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

Rules and card-source registry: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULE_SOURCES.md
