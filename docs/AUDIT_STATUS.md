# Audit Status

## Current source and test inventory

- Core exact-state policy fixtures: **57** in `regidrago_policy_tests`. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191
- Tier Two choice-differentiation fixtures: **31** in `regidrago_tier2_tests`. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73
- Deterministic scenario trace regressions: six CTest cases. The registered tests are https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L219-L224
- Aggregate scenario smoke test: one CTest case. The registered test is https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L225-L226

The detailed fixture names and executable evidence remain indexed in:

- Core policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/OPTIMAL_POLICY_FIXTURES.md
- Tier Two policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/TIER2_POLICY_FIXTURES.md
- Card-data provenance audit: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md

## Build and CI evidence

Pull requests run the card-audit provenance contract, a Release build with the full CTest suite, and a Debug build with AddressSanitizer and UndefinedBehaviorSanitizer. The workflow definition is https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

Both compiled lanes upload their CTest directories and build logs when validation succeeds or fails, so test conclusions can be checked against exact branch artifacts. The artifact configuration is https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L38-L46 and https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L68-L76

The inventory counts above are checked by `tests/card_audit_contract_tests.py` against the canonical runner tables and fixture-index documents. That contract runs in CI before either compiled lane: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L10-L18

## Remaining explicit model boundary

`FullRuleBoxAbility` is a scenario-level abstraction for a Path-to-the-Peak-style lock. Forest Seal Stone remains usable by an attached Pokémon V because the published ruling attributes Star Alchemy to the Tool rather than the Pokémon's own Ability: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
