# Debug-mode CI audit — 2026-08-08

Behavior-neutral trigger for a current-main pull-request CI audit. No simulator, policy, test, matrix, or rules source is changed by this branch.

The permanent CI workflow is expected to build the exact current production source, run the strict C++20 build, complete Release and sanitizer suites, execute its independent `--simulate-this` trace audits, and regenerate/compare canonical and paired setup matrices.

This audit branch is not intended for merge.
