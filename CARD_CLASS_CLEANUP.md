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
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts and reusable intrinsic classification. `CardDefinitionPredicates` centralizes kind, Trainer-subtype, and Pokémon-type tests. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific strategic route policy stays outside that interface. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/rules/card_context.hpp
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns the trace-engine bridge for reusable card effects. `src/trace_engine_v2/core/card_context_adapter.hpp` remains a compatibility include until direct consumers move.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated name and intrinsic-classification fallbacks. Registry lookup remains the first metadata path.
- `src/trace_engine_v2/core/payload_hand_policy.inc` owns shared Dragon-payload zone and preference queries. Route-specific DCI/JIT admission remains outside its reusable policies.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable board traversal and board-index queries.
- `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-hand, mulligan, Prize-deal, and setup-trace mechanics.
- `src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn action-state reset semantics.
- `src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after visibility is resolved. Hidden-zone visibility and route admission remain Engine concerns.
- `src/trace_engine_v2/core/tate/package.inc` owns the established discard-provenance, Tate attachment, and Tate action override order. Lower-level provenance and attachment bodies live under `core/tate/`.
- `src/trace_engine_v2/core/routes/search_connector_helpers.inc` owns complete K1 fallback selectors and traversal for Mysterious Treasure, Quick Ball, and Ultra Ball. Route-specific target priority remains separate.
- `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc` owns strategic Mysterious Treasure target priority. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- `src/trace_engine_v2/core/forretress/` owns reusable Forretress scenario and runtime composition, with `package.inc` as the namespace composition owner.

## Catalog and knowledge cleanup

Keep intrinsic catalog facts in `src/trace_engine_v2/core/card_catalog.inc` only while they remain unmigrated, and keep copy arithmetic in `src/trace_engine_v2/core/deck_knowledge.inc` after visibility is resolved. New card metadata should prefer the registry path, and hidden-zone visibility decisions stay in Engine strategy.

## Payload policy cleanup

Keep shared Dragon-payload zone and preference queries in `src/trace_engine_v2/core/payload_hand_policy.inc`. Route-specific strict-JIT timing, discard admission, deadlines, and setup-axis decisions stay with their strategy owners rather than moving into the reusable payload policy.

## Setup lifecycle cleanup

Keep opening-hand, mulligan, Prize-deal, and setup-trace mechanics in `src/trace_engine_v2/core/setup_lifecycle.inc`. Route admission and strategic opening choices stay outside the lifecycle owner.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The Steven/Brilliant Blender macro-composition block is directly owned by `src/trace_engine_v2/composition/steven_blender_overrides.inc` at the established post-`part_009b2.inc` boundary. The canonical owner receives the intentionally live aliases and releases them before later composition continues. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc

The source-bounded Steven route package has its canonical organized owner at `src/trace_engine_v2/core/routes/steven/package.inc`. The historical `src/trace_engine_v2/core/routes/steven_package_policy.inc` forwarding path is retired and should not be recreated. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

The late-Steven route body has its canonical policy owner at `src/trace_engine_v2/core/routes/late_steven_route_policy.inc`, and the Steven/Blender composition includes that owner directly. The historical `src/trace_engine_v2/part_010_late_steven_override.inc` source-contract mirror is retired and should not be recreated. Audit and traceability anchors must use the canonical route path. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven_route_policy.inc Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

The Steven/Brilliant Blender semantic route owners are `src/trace_engine_v2/core/routes/k0_steven_blender_semantic_policy.inc` and `src/trace_engine_v2/core/routes/steven_blender_semantic_policy.inc`. Their historical root compatibility includes are retired. Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164 Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

The Battle Compressor / VS Seeker route is owned directly by `src/trace_engine_v2/core/routes/battle_compressor_vs_seeker_policy.inc`; the historical root forwarding include is retired. Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92 VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109

The Hisuian Heavy Ball route body is owned by `src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc`. Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146

The Tate package composes discard/recovery provenance and proactive attachment from named owners under `core/tate/`. The remaining Tate action body is the next root-fragment migration at its identical member boundary.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Shared policy owners

Before adding a route-local loop or helper, reuse an existing owner when ordering and semantics match exactly:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and board indices: `src/trace_engine_v2/core/board_state_policy.inc`.
- Setup lifecycle: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.
- Intrinsic exact-print predicates: `src/cards/card_definition.hpp` via `CardDefinitionPredicates`.
- Optional card-effect classifier dispatch: `src/rules/card_context.hpp` via `CardContext::classify`.
- Shared post-search K1 fallback orders and traversal: `src/trace_engine_v2/core/routes/search_connector_helpers.inc` via `SearchConnectorFallbackPolicy`.
- Mysterious Treasure strategic target priority: `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc` via `MysteriousTreasureTargetPolicy`.
- Forretress registry and runtime composition: `src/trace_engine_v2/core/forretress/`.

## Next cleanup steps

1. Keep `composition/steven_blender_overrides.inc`, `core/routes/steven/package.inc`, and `core/routes/late_steven_route_policy.inc` as the canonical Steven organization boundaries. The historical late-Steven mirror is retired; keep future audit and traceability anchors on the canonical path.
2. Continue retiring composition-only `part_*steven*` forwarders when a complete function body or macro lifetime can move at the identical textual boundary. Do not recreate the retired issue-3221 or issue-3222 semantic forwarders.
3. Migrate direct `CardContext` bridge consumers to `core/adapters/card_context_adapter.hpp`. Remove the old forwarding include only after references are proven gone. New bridge construction should use `CardContextAdapterCallbacks`.
4. Migrate remaining `LegacyCardCatalog` and intrinsic compatibility rows one card at a time. Reuse `CardDefinitionPredicates` before adding raw metadata checks.
5. Continue replacing route-local board traversals with `core/board_state_policy.inc` only when first-match and ordering semantics are identical.
6. Keep Forretress reusable scenario and runtime ownership under `core/forretress/`; migrate the remaining historical consumer before deleting its last compatibility forwarder.
7. Keep the Tate package as the sole composition include. Migrate the remaining Tate action body at the identical member boundary, then retire historical mirrors after source-contract anchors move.
8. Prefer named pure-projection members when a projection is reused or has a distinct policy contract.
9. Prefer final stateless policy classes for reusable arithmetic or traversal seams. Keep route admission, hidden-zone visibility, DCI/JIT timing, and target preference in strategy owners.
10. Continue moving the remaining Mysterious Treasure member body out of `part_009a.inc` only when the full continuation can migrate atomically without changing declaration order. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

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