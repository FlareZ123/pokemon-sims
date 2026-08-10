# Debug-mode CI audit, 2026-08-10

Validation-only marker used to run the permanent repository CI from the current `main` snapshot during repository-wide debug review. This PR changes no simulator, policy, deck, test, or generated-result logic and should not be merged.

The permanent CI's `--simulate-this` trace audits, canonical/paired setup matrices, Release suite, and ASan/UBSan results are the evidence target for this audit.
