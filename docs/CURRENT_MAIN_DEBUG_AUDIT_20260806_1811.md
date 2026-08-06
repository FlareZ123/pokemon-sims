# Current-main debug audit

This documentation-only branch triggers permanent pull-request CI for an independent audit of `main@e7f7cc8587b97795239b1f9b8f0757d4ff6c031b`.

Requested validation:

- complete Release, strict C++20, and ASan/UBSan suites;
- at least three repository-defined `--simulate-this` passes with trace inspection;
- canonical and paired fixed-seed T2/T3 probability matrices;
- source-bound manifests and simulator policy digests.

All open bugs were reviewed before debug mode. Each has two approvals, an active claim, and fresh branch or pull-request work. This marker changes no simulator policy, card model, deck recipe, selector, probability logic, or rule interpretation.
