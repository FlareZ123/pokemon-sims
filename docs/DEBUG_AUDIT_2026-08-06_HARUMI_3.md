# Current-main debug audit marker

This documentation-only branch triggers permanent pull-request CI for an independent review of `main@0206c21c4835e942d6842289bcbbb7e23841b45b`.

Requested validation:

- complete Release, strict C++20, and ASan/UBSan suites;
- at least three repository-defined `--simulate-this` passes with trace inspection;
- canonical and paired fixed-seed T2/T3 probability matrices;
- source-bound manifests, policy digests, and trace artifacts.

Open-bug triage before debug mode:

- #2164 has fresh branch and PR activity: https://github.com/FlareZ123/pokemon-sims/issues/2164 https://github.com/FlareZ123/pokemon-sims/pull/2187
- #2165 has fresh branch and PR activity: https://github.com/FlareZ123/pokemon-sims/issues/2165 https://github.com/FlareZ123/pokemon-sims/pull/2192
- #2172 remains associated with active PR review and stacked dependencies: https://github.com/FlareZ123/pokemon-sims/issues/2172 https://github.com/FlareZ123/pokemon-sims/pull/2174
- #2188, #2189, #2190, and #2191 were independently re-verified against current `main` and received one approval each. They remain unavailable until a second agent approves the latest wording: https://github.com/FlareZ123/pokemon-sims/issues/2188 https://github.com/FlareZ123/pokemon-sims/issues/2189 https://github.com/FlareZ123/pokemon-sims/issues/2190 https://github.com/FlareZ123/pokemon-sims/issues/2191

This marker changes no simulator policy, card model, deck recipe, selector, or probability logic.