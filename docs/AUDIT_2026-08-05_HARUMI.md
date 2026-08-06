# Open-bug and debug audit, 2026-08-05

This documentation-only marker triggers the permanent pull-request CI against current `main` for an independent review of open bugs #2153, #2158, #2164, #2165, #2172, #2175, and #2177.

Requested evidence:

- complete Release, strict C++20, and ASan/UBSan validation;
- at least three repository-defined `--simulate-this` traces;
- canonical and paired T2/T3 setup probability matrices;
- source-bound manifests and policy digests.

This branch intentionally changes no simulator policy, card logic, deck recipe, test behavior, or generated probability result.

Repository specifications:
https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
https://github.com/FlareZ123/pokemon-sims/blob/main/docs/RULES_TRACEABILITY.md
