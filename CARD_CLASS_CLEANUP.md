# Card Class Cleanup

This file is the live architecture and migration plan. Historical cleanup-wave notes belong in Git history. Keep this document limited to current ownership, remaining work, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Bootstrap gate

Do not begin another card migration unless the Quick Ball reference seam remains intact:

```text
src/cards/card_id.hpp
src/cards/card_definition.hpp
src/cards/card_registry.hpp
src/cards/trainers/quick_ball.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/adapters/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokemon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` is the sole trace-engine construction bridge for reusable card effects. The former `src/trace_engine_v2/core/card_context_adapter.hpp` forwarding include is retired after repository reference search and CI validation.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` remains the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains the first metadata path.

Next catalog step: migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time. Delete a compatibility row only after that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

Regidrago VSTAR owns exact Silver Tempest 136/195 metadata beside Regidrago V in `src/cards/pokemon/regidrago_v.hpp`, is explicitly registered, and has focused V/VSTAR metadata/parity coverage. Exact prints: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Pokemon V ruling: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata/classification can move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata/classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow instead of combining the fix with cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It owns the simulator runtime inclusion, opening `part_003.inc` -> `part_004.inc` -> `part_005.inc` continuation, banked-Tapu and lock-removal alias lifetimes, and late `part_014c.inc` -> `part_015.inc` -> `part_016.inc` continuation. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc Runtime state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

`src/trace_engine_v2/composition/steven_blender_overrides.inc` owns the contiguous Steven/Brilliant Blender macro-composition block formerly embedded in `opening_engine_overrides.inc`. Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc

The root `part_000.inc` and `part_001.inc` paths remain source-contract shims. `part_000.inc` is the single legacy catalog include owner. `part_001.inc` now includes that shim directly and relies on `part_000.inc` for the catalog guard, while preserving the non-executable payload predicate mirror and historical line-137 anchor. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Unified-test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

The issue-1393 Crispin route, issue-1516/2164 Quick Ball/Tapu/Crispin route family, and issue-1368 Earthen Vessel/Celestial Roar route have canonical owners under `src/trace_engine_v2/core/routes/`. Preserve route admission, copied-state projection, K1 checks, lock checks, DCI/JIT timing, trace text, and direct rule/card sources when retiring root compatibility seams.

Next composition step: prove raw-source/tooling consumers of `part_issue_1368_earthen_vessel_celestial_roar_override.inc` are absent, rewire the post-`part_014a` composition boundary directly to `core/routes/earthen_vessel_celestial_roar_policy.inc`, then retire the root compatibility include. Inspect another root `part_*` seam only when its complete macro lifetime or function body can move intact.

### Banked Tapu paid-retreat seam

`src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc` owns the paid-retreat priority gate and deterministic physical Basic Energy payment for Tapu Lele-GX. Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60 Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76 Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148 Advanced Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Keep route strategy separate from generic Retreat procedure and card validation.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse `PayloadZonePolicy` for physical-zone first-match, membership, cardinality, and concrete-card checks. Reuse `PayloadPreferencePolicy` for explicit strategic priority and count-backed preference traversal.

Next payload step: audit remaining ad hoc Dragon-payload scans and selectors. Replace them only when semantics exactly match an existing policy operation. Preserve physical order where observable, strategic order where intentional, and DCI/JIT timing at strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine declarations and card-facing board-role classifiers. `src/trace_engine_v2/core/forretress/runtime.inc` owns the Forretress runtime. `src/trace_engine_v2/core/forretress/scenario_extension.inc` owns reusable scenario storage. `src/trace_engine_v2/core/forretress/garbodor_scenario_extension.inc` owns Garbodor / Boost Shake rows and append/lookup entry points. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142

Next mechanical Forretress step: inventory remaining orchestration in `runtime.inc` and adjacent root route fragments for another complete semantic boundary. Preserve state-count queries, entry-turn evolution timing, route ordering, attachment distribution, retreat planning, and strategic ranking at their existing owners.

## Steven route cleanup

Named Steven route policies live under `src/trace_engine_v2/core/routes/`. `core/routes/gladion_steven_route_policy.inc` owns the shared `resolve_gladion_prize_exchange()` state transition. Projected Item-lock timing delegates to Engine `item_locked_on_turn()` instead of re-encoding lock-family identities inside route files. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Gladion: https://api.pokemontcg.io/v2/cards/sm4-95 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next Steven cleanup step: migrate duplicated Gladion Prize-exchange mutations only when reveal and target-selection semantics match exactly. Continue retiring composition-only `part_*steven*` forwarders at identical textual boundaries.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing labels, opening-deck initialization, opening hand and mulligan mechanics, Prize dealing, and setup trace output. `SetupRecipePolicy` owns setup recipe-presence and exact-count predicates. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

Next setup step: route future setup recipe classification through `SetupRecipePolicy`. Move setup state-transition helpers only with exact source-contract coverage for hand removal, `started_regi`, Bench insertion, and declaration ordering.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name bridge and intrinsic compatibility seam. `src/trace_engine_v2/core/deck_knowledge.inc` owns shared copy arithmetic through `KnowledgeCopyPolicy`. K0/K1 visibility rules remain at Engine callers: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: migrate legacy metadata rows only after explicit registration and coverage. Move copy arithmetic into `KnowledgeCopyPolicy` only after visibility has been resolved by the Engine caller. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

## Turn lifecycle cleanup

`src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn resets. `TurnActionStatePolicy::reset()` clears generic action flags and same-turn discard tracking. `TransientTurnLockPolicy::reset()` owns scenario-dependent one-turn Garbodor unlock reset. Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104 Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next turn-lifecycle step: route exact duplicate action-flag/reset bundles through the existing policy owners. Preserve ordering relative to the required turn draw and keep persistent matchup state outside per-turn owners.

## Projection cleanup

Prefer named pure-projection members over route-local anonymous lambdas when a projection is reused or carries a distinct policy contract. Merge a remaining root fragment into a canonical semantic owner only when its complete function body or macro lifetime can move at the identical textual boundary. Keep physical resolution, trace emission, K0/K1 transitions, strategic route choice, and source URLs at their current owners.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
