# Issue 1821 validation

The confirmed seed-161803 Steven, Latias ex, and Grass Energy route was validated after composing pull request #1826 with current `main` commit `df70c43f039b3042f0781a65af327c82f9a4c548`.

The source-bound validation passed the focused regression, three independent `--simulate-this` traces, regenerated 100,000-trial canonical and paired matrices, the complete Release suite, and the complete AddressSanitizer and UndefinedBehaviorSanitizer suite.

Validation run: https://github.com/FlareZ123/pokemon-sims/actions/runs/30478627406
Confirmed issue: https://github.com/FlareZ123/pokemon-sims/issues/1821
Implementation pull request: https://github.com/FlareZ123/pokemon-sims/pull/1826
