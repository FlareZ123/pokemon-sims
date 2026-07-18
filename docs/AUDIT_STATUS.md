# Audit Status

## Current source and test inventory

- Core exact-state policy fixtures: **57** in the `regidrago_unified_tests regidrago_policy_tests` case. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191
- Tier Two choice-differentiation fixtures: **31** in the `regidrago_unified_tests regidrago_tier2_tests` case. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73
- Deterministic scenario trace regressions: six CTest cases registered in https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
- Aggregate scenario smoke test: one CTest case registered in https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt

The detailed fixture names and executable evidence remain indexed in:

- Core policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/OPTIMAL_POLICY_FIXTURES.md
- Tier Two policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/TIER2_POLICY_FIXTURES.md
- Card-data provenance audit: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md

## Build and CI evidence

Pull requests run two compiled lanes from https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

The Release lane builds the simulator and unified tests, runs three readable `--simulate-this` audits, generates the canonical 100,000-trial matrix, and then runs the complete CTest suite. It uploads the three traces, matrix, tested commit identifier, and CTest evidence directory.

The sanitizer lane builds with AddressSanitizer and UndefinedBehaviorSanitizer, runs the complete CTest suite, and uploads its CTest evidence directory. Card-audit and documentation contracts run inside both complete CTest suites after compilation.

## Remaining explicit model boundary

`FullRuleBoxAbility` is a scenario-level abstraction for a Path-to-the-Peak-style lock. Forest Seal Stone remains usable by an attached Pokémon V because the published ruling attributes Star Alchemy to the Tool rather than the Pokémon's own Ability: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
