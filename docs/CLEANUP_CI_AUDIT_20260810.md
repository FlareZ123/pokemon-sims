# Cleanup CI audit, 2026-08-10

This PR-only marker triggers the permanent repository CI against `main@f9681050271e89fc7681bd12084ecff616d5e872` after the three direct composition cleanup commits. The permanent CI runs strict C++20 compilation, eight `--simulate-this` audits, Pineco trace audits, canonical and paired 100,000-trial matrix checks, the complete Release suite, and ASan/UBSan.

The marker is validation-only and should not be merged.
