# Issue 1700 validation

The confirmed seed-218 Earthen Vessel, Steven's Resolve, Gladion, Latias ex, and Brilliant Blender route was validated after composing pull request #1833 with current `main` after pull request #1826.

The source-bound validation passed both focused regressions, three individual `--simulate-this` traces, regenerated 100,000-trial canonical and paired reports, all 350 Release tests, the complete AddressSanitizer and UndefinedBehaviorSanitizer suite, and final 100,000-trial shell and all-condition T2/T3 matrices.

Validated source commit: https://github.com/FlareZ123/pokemon-sims/commit/114a4d7175c8ae631b4a9183bdb951d897df4cd7

Validation run: https://github.com/FlareZ123/pokemon-sims/actions/runs/30481885743

Confirmed issue: https://github.com/FlareZ123/pokemon-sims/issues/1700

Implementation pull request: https://github.com/FlareZ123/pokemon-sims/pull/1833

Card sources: https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/sm4-95 https://api.pokemontcg.io/v2/cards/sv8-76 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/swsh12-136

Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
