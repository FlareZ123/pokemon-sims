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
src/trace_engine_v2/core/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box, Pokemon V, ACE SPEC, and Basic Energy flags.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and `find_definition()`: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns trace-engine construction of reusable card effects through `CardContextAdapterCallbacks`.
- `src/trace_engine_v2/core/card_context_adapter.hpp` remains a forwarding include until repository-wide direct consumers are migrated.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT, matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registered metadata remains the first lookup path.

Next adapter step: migrate direct consumers of `core/card_context_adapter.hpp` to `core/adapters/card_context_adapter.hpp`, then remove the forwarding include only after repository-wide references are proven gone.

Next catalog step: migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time. Delete a compatibility row only after the card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata and classification can move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata and classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow instead of combining the fix with cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It preserves the runtime inclusion and the behavior-significant member/include order. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

`src/trace_engine_v2/composition/steven_blender_overrides.inc` is now an ordering spine at the historical post-`part_009b2.inc` boundary. It receives the intentionally live `play_ultra_ball` alias, checks entry and exit alias state, and composes three complete macro families in order:

1. `src/trace_engine_v2/composition/steven_search_overrides.inc` owns the base `part_010.inc` search aliases and the late-Steven continuation. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156 Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
2. `src/trace_engine_v2/composition/steven_vstar_vessel_overrides.inc` owns the issue-3202 wrapper lifetime and composes the canonical VSTAR/Vessel route at the same member boundary. Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Canonical route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_vstar_vessel_route_policy.inc
3. `src/trace_engine_v2/composition/blender_thinning_overrides.inc` owns the Brilliant Blender thinning wrapper and releases the search aliases at their historical boundary. Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164

These are composition-only extractions. They do not move route admission, hidden-information logic, DCI/JIT policy, or printed resolution. Advanced procedure source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The root `part_000.inc` and `part_001.inc` compatibility paths remain because unified-test and source-contract tooling reads them directly. `part_000.inc` is the legacy catalog include shim and `part_001.inc` delegates catalog inclusion through it while preserving its raw-source contract. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc

Named route owners remain under `src/trace_engine_v2/core/routes/`. Important established owners include:

- held-Crispin completion: `crispin_supported_route_policy.inc`
- Quick Ball / Tapu Lele-GX / Crispin: `quick_ball_tapu_crispin_policy.inc`
- Earthen Vessel / Celestial Roar: `earthen_vessel_celestial_roar_policy.inc`
- K0 Steven / Brilliant Blender: `k0_steven_blender_semantic_policy.inc`
- banked Tapu paid Retreat: `banked_tapu_retreat_policy.inc`
- Steven / Regidrago VSTAR / Earthen Vessel: `steven_vstar_vessel_route_policy.inc`

Keep direct card, rule, ruling, specification, and issue URLs beside rule-sensitive production logic when any owner moves.

Next composition step: inspect another root `part_*` seam only when its complete macro lifetime or complete function body can move intact to a named composition or semantic owner. Prefer removing forwarding fragments over introducing new compatibility layers. Preserve alias lifetime, declaration order, trace behavior, route semantics, and direct source URLs.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. `PayloadZonePolicy` owns physical-zone membership, cardinality, and first-match traversal; `PayloadPreferencePolicy` owns explicit strategic preference traversal. Route-local DCI/JIT admission and discard timing remain with strategy owners. Canonical payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

Next payload step: replace ad hoc Dragon-payload scans only when physical-order and preference semantics exactly match an existing payload policy operation.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup labels, opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that owner at the established Engine member boundary. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

Next setup step: reuse setup recipe predicates where semantics match exactly and move state-transition helpers only with source-contract coverage for hand removal, Active/Bench placement, and declaration order.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name and intrinsic-classification compatibility seam. Registered `CardDefinition` lookup remains canonical for migrated metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

`src/trace_engine_v2/core/deck_knowledge.inc` owns copy arithmetic after the Engine caller has resolved visibility. K0/K1 visibility, Prize deduction, and search timing remain strategy concerns. Knowledge-state specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: move repeated copy-count arithmetic into the knowledge owner only after visibility is resolved, and retire legacy catalog rows only after explicit card registration and focused coverage.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and entry-turn predicates: `src/trace_engine_v2/core/board_state_policy.inc`.
- Deck copy arithmetic after visibility is resolved: `src/trace_engine_v2/core/deck_knowledge.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Per-turn reset state: `src/trace_engine_v2/core/turn_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.
- Forretress runtime and scenario-family helpers: `src/trace_engine_v2/core/forretress/`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam only when ordering and semantics match exactly.

## Remaining cleanup priorities

1. Finish direct `CardContext` adapter include migration and retire its forwarding header after reference proof.
2. Continue one-card metadata migration without moving strategy into card modules.
3. Retire composition-only `part_*` forwarders when a complete macro lifetime can move at the identical textual boundary.
4. Reuse payload, board, knowledge, setup, turn-lifecycle, and Forretress owners only when their visibility and ordering semantics match the caller exactly.
5. Prefer pure copied-Engine projections over temporary mutation of live state when a route needs hypothetical evaluation.
6. Keep hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission in strategy owners.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer and structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering and readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
