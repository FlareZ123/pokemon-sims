# Agent audit trigger, 2026-08-02

This documentation-only marker triggers the permanent pull-request CI on current `main` for a repository-wide bug review.

The requested validation surfaces are:

- complete Release tests;
- complete ASan/UBSan tests;
- the repository-defined independent `--simulate-this` traces;
- canonical 100,000-trial T2/T3 setup matrices;
- paired registered-deck matrices and uploaded evidence.

This file changes no simulator behavior and should be removed by closing the audit PR without merging after evidence inspection.
