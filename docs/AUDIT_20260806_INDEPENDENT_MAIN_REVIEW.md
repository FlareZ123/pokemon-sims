# Independent current-main CI audit

This branch changes no simulator policy, card logic, deck recipe, or production behavior. It exists only to trigger the permanent pull-request validation workflow against the current `main` branch during an independent open-bug and debug-mode review.

Validation scope:

- complete Release suite;
- strict C++20 build contract;
- complete ASan/UBSan suite;
- all permanent `--simulate-this` audit traces, including at least three independently reviewed traces;
- canonical 100,000-trial T2/T3 setup matrix;
- paired 100,000-trial registered-deck matrix;
- source-bound simulator policy digest and tested-head evidence.

Open issues reviewed before this audit: #2164, #2165, #2199, #2200, #2201, #2202, #2203, #2204, and #2205. Each currently has a fresh claim plus active branch/PR work, so this audit does not claim, modify, or supersede those issues.

Close this pull request without merging after its CI artifacts and traces are inspected.
