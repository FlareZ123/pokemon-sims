# Current-main CI audit — 2026-08-07

This documentation-only marker requests an independent validation of the current `main` simulator without changing card logic, route policy, deck recipes, or production behavior.

Validation scope:

- permanent `--simulate-this` audits, with at least three traces independently reviewed for earliest legal play;
- complete Release and sanitizer suites;
- canonical 100,000-trial T2/T3 setup matrix;
- paired registered-deck 100,000-trial matrix;
- tested-head and simulator-policy provenance.

This audit branch is intended to be closed without merging after evidence is inspected.
