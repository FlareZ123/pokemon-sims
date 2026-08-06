# Current-main debug audit marker

This documentation-only branch exists to trigger permanent pull-request CI for an independent review of `main` at `fbb05bf1c5e18d9b326251316b940719efbc487a`.

Requested validation:

- complete Release, strict C++20, and ASan/UBSan suites;
- at least three repository-defined `--simulate-this` passes;
- canonical and paired fixed-seed T2/T3 probability matrices;
- source-bound manifests, policy digests, and trace artifacts.

Open bugs reviewed before debug mode: #2164, #2165, #2172, and #2175. All had active claims and fresh branch or PR activity within twelve hours.

This marker changes no simulator policy, card model, deck recipe, selector, or probability logic.
