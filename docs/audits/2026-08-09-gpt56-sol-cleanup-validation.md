# GPT-5.6 Sol cleanup validation

This validation-only branch exists to run the repository's permanent pull-request CI against the current `main` state after three direct-main cleanup refactors.

Validation scope:

- compile and test the simulator;
- run the permanent `--simulate-this` trace audits;
- verify setup probability evidence and provenance contracts.

No gameplay policy is changed by this branch.
