# Current-main debug audit marker

This documentation-only branch triggers permanent pull-request CI for an independent review of `main@3f297814c22283be9f9b435d563f8e735e834062`.

Requested validation:

- complete Release, strict C++20, and ASan/UBSan suites;
- at least three repository-defined `--simulate-this` passes with trace inspection;
- canonical and paired fixed-seed T2/T3 probability matrices;
- source-bound manifests, policy digests, and trace artifacts.

Open-bug triage before debug mode:

- #2164 remains actively claimed with an open implementation PR: https://github.com/FlareZ123/pokemon-sims/issues/2164 https://github.com/FlareZ123/pokemon-sims/pull/2167
- #2165 remains actively claimed with open implementation PRs: https://github.com/FlareZ123/pokemon-sims/issues/2165 https://github.com/FlareZ123/pokemon-sims/pull/2169 https://github.com/FlareZ123/pokemon-sims/pull/2192
- #2191 remains actively claimed with a current-main implementation PR: https://github.com/FlareZ123/pokemon-sims/issues/2191 https://github.com/FlareZ123/pokemon-sims/pull/2198
- #2199 through #2205 each have one independent approval and no claim. Current-main CI traces from this audit will be used to re-verify their exact latest wording before any second approval or claim: https://github.com/FlareZ123/pokemon-sims/issues/2199 https://github.com/FlareZ123/pokemon-sims/issues/2200 https://github.com/FlareZ123/pokemon-sims/issues/2201 https://github.com/FlareZ123/pokemon-sims/issues/2202 https://github.com/FlareZ123/pokemon-sims/issues/2203 https://github.com/FlareZ123/pokemon-sims/issues/2204 https://github.com/FlareZ123/pokemon-sims/issues/2205

This marker changes no simulator policy, card model, deck recipe, selector, or probability logic.
