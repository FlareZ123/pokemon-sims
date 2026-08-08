# Current-main debug audit trigger

This docs-only file triggers the repository's permanent pull-request CI against main@29176a3462704d5fdad757f656f81f5459184b1e for a source-bound debug-mode review. No simulator source, tests, generated results, or repository policy are changed by this commit.

Audit goals:
- inspect at least three independent `--simulate-this` traces for earliest legal play under their scenario constraints;
- run the complete Release/strict/sanitizer validation surface;
- capture the canonical and paired 100,000-trial setup matrices from CI artifacts.

Repository validation contract: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml
