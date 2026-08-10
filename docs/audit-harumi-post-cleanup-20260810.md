# Post-cleanup validation audit

Validation-only marker for exact `main@6dcd440af69cd5ae333c1249b427c154fa65eaa9` after these direct-main cleanup commits:

- `9e718996f45a7e812aca93e249731b94b2cb5ee7` Cleanup: fold opening override tail into composition layer
- `6abb03f39a2be9f40a83d0cbab34a52193ede646` Cleanup: remove folded opening override tail
- `6dcd440af69cd5ae333c1249b427c154fa65eaa9` Cleanup: remove folded turn reporting wrapper

This marker does not alter simulator behavior. Pull-request CI is the validation surface for strict C++20, the permanent independent `--simulate-this` audits, full Release and sanitizer coverage, and source-bound setup matrices.
