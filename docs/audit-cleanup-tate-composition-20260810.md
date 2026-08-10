# Cleanup composition CI audit

Validation-only marker for `main@6f0302e49a53181c1d7d4c92565dae68d4928d8b` after the three direct cleanup commits that consolidated the legacy Tate `.inc` composition. This marker does not change simulator behavior and must not be merged.

CI should validate strict compilation, permanent `--simulate-this` traces, sanitizer coverage, and the fixed-seed setup matrix contracts against the cleaned source tree.
