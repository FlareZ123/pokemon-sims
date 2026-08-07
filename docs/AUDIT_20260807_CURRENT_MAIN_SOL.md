# Current-main debug audit — 2026-08-07

This branch exists only to trigger pull-request CI against current `main` during an independent bug-queue and debug-mode review. It changes no simulator policy, card logic, deck recipe, or production behavior.

Validation scope:

- complete Release and strict C++20 validation;
- complete ASan/UBSan validation;
- all permanent `--simulate-this` trace audits, with at least three traces manually reviewed for optimal legal play;
- canonical and paired 100,000-trial T2/T3 setup matrices;
- tested-head and simulator-policy provenance evidence.

The audit branch is intended to be closed without merging after its CI evidence is inspected.
