# Production-neutral debug audit

This branch exists only to trigger the repository's permanent pull-request CI against the current `main` snapshot for the 2026-08-08 23:09 ET audit.

Validation target:
- strict C++20 build
- complete Release and sanitizer suites
- permanent `--simulate-this` trace audits
- canonical 100,000-trial T2/T3 matrix
- paired 100,000-trial multi-deck matrix

No simulator, policy, deck, test, or generated-result logic is changed by this marker.
