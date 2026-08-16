# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup details remain in Git history. Keep this file focused on current ownership, remaining boundaries, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Bootstrap gate

Keep the Quick Ball reference seam intact while other cards migrate:

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

## Current ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box, Pokemon V, ACE SPEC, Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific strategic route policy stays outside that interface.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns the trace-engine bridge for reusable card effects. `src/trace_engine_v2/core/card_context_adapter.hpp` is a compatibility include until direct consumers move.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated name and intrinsic-classification fallbacks. Registry lookup remains the first metadata path.
- `src/trace_engine_v2/core/payload_hand_policy.inc` owns shared Dragon-payload zone and preference queries.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable board traversal and board-index queries.
- `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-hand, mulligan, Prize-deal, and setup-trace mechanics.
- `src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn action-state reset semantics.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The Steven/Brilliant Blender macro-composition block now has an organized canonical owner at `src/trace_engine_v2/composition/steven/blender_overrides.inc`. The historical `src/trace_engine_v2/composition/steven_blender_overrides.inc` path is a thin compatibility include at the identical post-`part_009b2.inc` boundary. The canonical owner receives the intentionally live `play_ultra_ball` alias and releases the same search, Steven, and Blender aliases before later composition continues. This is a textual ownership move. Route admission remains with `core/routes/`. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven/blender_overrides.inc

The source-bounded Steven route package now has a canonical organized owner at `src/trace_engine_v2/core/routes/steven/package.inc`. The historical `src/trace_engine_v2/core/routes/steven_package_policy.inc` path is a thin compatibility include. The package preserves the established #1745 -> #1771 -> #1772 -> #2622 textual route order while rule-sensitive function bodies remain in their existing route owners. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Canonical package: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven/package.inc

The late-Steven route body now has a canonical policy owner at `src/trace_engine_v2/core/routes/late_steven_route_policy.inc`. The historical `src/trace_engine_v2/part_010_late_steven_override.inc` path is a thin compatibility include at the same Engine member boundary, so #869/#3601 route admission and continuation stay textually ordered while the implementation lives with other route policies. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven_route_policy.inc Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

The namespace-scope Forretress composition now has a canonical package at `src/trace_engine_v2/core/forretress/package.inc`. The historical `src/trace_engine_v2/part_forretress_ex_combo.inc` path is a thin compatibility include. The package keeps the reusable runtime, public scenario forward declarations, shared scenario extension, and Garbodor scenario family at the same verified namespace boundary while preserving their textual order. Canonical package: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/package.inc C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The issue-3221 K0 Steven/Brilliant Blender route remains owned by `src/trace_engine_v2/core/routes/k0_steven_blender_semantic_policy.inc`. Its historical root compatibility seam remains until source-contract and composition consumers are proven migrated. Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164 Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The issue-1368 Earthen Vessel / Celestial Roar route remains owned by `src/trace_engine_v2/core/routes/earthen_vessel_celestial_roar_policy.inc`, which keeps route-specific target preference separate from shared Item and search legality. Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163 Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

The issue-1516/2164 Quick Ball, Tapu Lele-GX, Crispin route family remains owned by `src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc`. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60 Crispin: https://api.pokemontcg.io/v2/cards/sv7-133

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse `PayloadZonePolicy` only where physical-zone traversal, membership, and count semantics match exactly. Preserve physical order for observable first-match selection and preserve explicit strategic order for preference selection. DCI/JIT predicates and discard timing remain with strategy owners. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing labels, opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that owner at the established Engine member boundary. Preserve setup declaration order and hand, Active, Bench, and Prize transitions. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name bridge and intrinsic-classification compatibility seam. Registered `CardDefinition` lookup remains canonical for migrated metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

`src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after the Engine caller resolves visibility. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns. K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

## Shared policy owners

Before adding a route-local loop or helper, reuse an existing owner when ordering and semantics match exactly:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and board indices: `src/trace_engine_v2/core/board_state_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

## Next cleanup steps

1. Keep `composition/steven/blender_overrides.inc` and `core/routes/steven/package.inc` as the canonical Steven organization boundaries. Remove their forwarding paths only after repository-wide and source-contract references are proven gone.
2. Keep `core/routes/late_steven_route_policy.inc` as the canonical late-Steven policy body. Retire `part_010_late_steven_override.inc` only after composition and source-contract consumers no longer depend on the historical include seam.
3. Continue retiring other composition-only `part_*steven*` forwarders when a complete function body or macro lifetime can move at the identical textual boundary.
4. Migrate direct `CardContext` bridge consumers to `core/adapters/card_context_adapter.hpp`. Remove the old forwarding include only after references are proven gone. New bridge construction should use `CardContextAdapterCallbacks`.
5. Migrate remaining `LegacyCardCatalog` and intrinsic compatibility rows one card at a time. Delete a row only after explicit `CardDefinition` registration, exact-print source, and focused metadata coverage exist.
6. Reuse payload and board-state owners only when physical order, visibility, and strategic preference semantics match exactly.
7. Keep Forretress reusable scenario and runtime ownership under `core/forretress/`, with `core/forretress/package.inc` as the namespace-scope composition owner. Remove `part_forretress_ex_combo.inc` only after repository-wide and source-contract consumers are proven migrated.
8. Prefer named pure-projection members over route-local anonymous projections when a projection is reused or has a distinct policy contract.

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

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow and keep the behavior fix out of cleanup.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer and structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering and readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to an existing issue and remain unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow.