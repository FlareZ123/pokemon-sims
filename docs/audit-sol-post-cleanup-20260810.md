# Post-cleanup current-main audit

Behavior-neutral CI trigger for `main@5a52bc1d9252d1e18fcaeaa7f98a365dc65a07d2` after the three direct-main composition cleanups performed on 2026-08-10.

Expected parity with the pre-cleanup audit: Release and sanitizer compilation remain clean, permanent `--simulate-this` audits remain clean, and only the already-owned stale generated-result provenance contracts may fail until #2725 is refreshed. Close this PR without merge after evidence inspection.
