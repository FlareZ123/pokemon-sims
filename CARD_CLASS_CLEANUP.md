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
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts and reusable intrinsic classification. `CardDefinitionPredicates` centralizes kind, Trainer-subtype, and Pokémon-type tests while the established free functions remain compatibility seams. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Its optional intrinsic classifier callbacks share one null-safe dispatch seam. Card-specific strategic route policy stays outside that interface. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/rules/card_context.hpp
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns the trace-engine bridge for reusable card effects. `src/trace_engine_v2/core/card_context_adapter.hpp` is a compatibility include until direct consumers move.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated name and intrinsic-classification fallbacks. Registry lookup remains the first metadata path.
- `src/trace_engine_v2/core/payload_hand_policy.inc` owns shared Dragon-payload zone and preference queries. `PayloadZonePolicy` and `PayloadPreferencePolicy` are explicit final stateless policy classes so traversal and preference order stay centralized without becoming extensible Engine subtypes.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable board traversal and board-index queries.
- `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-hand, mulligan, Prize-deal, and setup-trace mechanics.
- `src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn action-state reset semantics.
- `src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after visibility is resolved. `KnowledgeCopyPolicy` is an explicit final stateless policy class; hidden-zone visibility and route admission remain Engine concerns.
- `src/trace_engine_v2/core/routes/oricorio_connector_policy.inc` owns the Oricorio connector's pure energy-need and connector-admission projections through `OricorioConnectorPolicy`; Engine retains Ability availability, K0/K1 visibility, Bench-space, and action execution. Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
- `src/trace_engine_v2/core/routes/issue_1016_legacy_star_quick_ball_policy.inc` owns the Legacy Star/Quick Ball pure connector and inertness projections through `LegacyStarQuickBallPolicy`; Engine retains DCI/JIT state queries and the temporary projection-only hand mutation around Legacy Star. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
- `src/trace_engine_v2/core/tate/package.inc` owns the established discard-provenance, Tate attachment, and Tate action override sequence as one composition package. The historical member fragments remain the rule-sensitive implementation owners while this package centralizes their textual order: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/package.inc
- `src/trace_engine_v2/core/routes/battle_compressor_vs_seeker_policy.inc` is the canonical Battle Compressor / VS Seeker route owner and is consumed directly by `composition/engine_body.inc`. The historical `src/trace_engine_v2/core/battle_compressor_vs_seeker_policy.inc` compatibility include is retired. Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92 VS Seeker: https://api.pokemontcg.io/v2/cards/xy4-109 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- `src/trace_engine_v2/core/routes/search_connector_helpers.inc` owns `need_regi`, `need_vstar`, payload-outlet checks, and the complete K1 fallback selectors for Mysterious Treasure, Quick Ball, and Ultra Ball. `SearchFallbackPolicy` is the final stateless owner for shared fallback traversal and the three explicit card-specific candidate orders; Engine callers retain K1 visibility and route decisions. `part_008b.inc` composes this owner at the same Engine member boundary, while `part_009a.inc` now starts with the remaining Mysterious Treasure route body. DCI/UDP/AMR and route-specific target choice remain Engine strategy concerns. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146 Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The Steven/Brilliant Blender macro-composition block is now directly owned by `src/trace_engine_v2/composition/steven_blender_overrides.inc` at the identical post-`part_009b2.inc` boundary. The redundant nested `src/trace_engine_v2/composition/steven/blender_overrides.inc` forwarding layer has been retired. The canonical owner receives the intentionally live `play_ultra_ball` alias and releases the same search, Steven, and Blender aliases before later composition continues. This remains a textual ownership cleanup. Route admission remains with `core/routes/`. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc

The source-bounded Steven route package now has a canonical organized owner at `src/trace_engine_v2/core/routes/steven/package.inc`. The historical `src/trace_engine_v2/core/routes/steven_package_policy.inc` path is a thin compatibility include. The package preserves the established #1745 -> #1771 -> #1772 -> #2622 textual route order while rule-sensitive function bodies remain in their existing route owners. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Canonical package: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven/package.inc

The late-Steven route body now has a canonical policy owner at `src/trace_engine_v2/core/routes/late_steven_route_policy.inc`, and the Steven/Blender composition includes that owner directly. The historical `src/trace_engine_v2/part_010_late_steven_override.inc` body remains temporarily as an exact source-contract mirror because existing audit and traceability anchors still reference its historical line numbers. It is no longer part of the composition path. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven_route_policy.inc Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

The namespace-scope Forretress composition now has a canonical package at `src/trace_engine_v2/core/forretress/package.inc`. The historical `src/trace_engine_v2/part_forretress_ex_combo.inc` path is a thin compatibility include. The package keeps the reusable runtime, public scenario forward declarations, shared scenario extension, and Garbodor scenario family at the same verified namespace boundary while preserving their textual order. Canonical package: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/package.inc C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The Steven/Brilliant Blender semantic route owners are `src/trace_engine_v2/core/routes/k0_steven_blender_semantic_policy.inc` and `src/trace_engine_v2/core/routes/steven_blender_semantic_policy.inc`. Their former issue-3221 and issue-3222 root compatibility includes are retired, and `src/trace_engine_v2/part_issue_1067_arven_before_late_steven_override.inc` now composes those canonical route owners directly at the same member boundary. Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164 Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The issue-1368 Earthen Vessel / Celestial Roar route remains owned by `src/trace_engine_v2/core/routes/earthen_vessel_celestial_roar_policy.inc`, which keeps route-specific target preference separate from shared Item and search legality. Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163 Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

The issue-1516/2164 Quick Ball, Tapu Lele-GX, Crispin route family remains owned by `src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc`. Its Latias completion projection delegates Bench traversal to `core/board_state_policy.inc` and Rule Box Ability admission to the canonical per-Pokémon Ability predicate, while route-specific JIT, evolution, connector, and action decisions remain in the route owner. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60 Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76 Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc Canonical Ability-lock owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

The Hisuian Heavy Ball route body is now owned by `src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc`. `src/trace_engine_v2/part_008b.inc` composes that complete member body at its existing Engine boundary before continuing into the shared search-connector helpers, so K0/K1 inspection timing, Prize recovery preference, Supporter-contention projections, and trace behavior remain in the same strategy layer. Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146 C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The complete search-connector preparation and post-search fallback helpers now have one named owner at `src/trace_engine_v2/core/routes/search_connector_helpers.inc`. `SearchFallbackPolicy` centralizes repeated first-live-target traversal and names the Mysterious Treasure, Quick Ball, and Ultra Ball K1 fallback candidate sets without moving visibility or route admission out of Engine. `part_008b.inc` includes that owner directly after the Heavy Ball policy, and the historical Quick Ball continuation plus adjacent Ultra Ball fallback have both moved out of numbered fragments without changing member order. The next numbered boundary begins with the Mysterious Treasure route in `part_009a.inc`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse `PayloadZonePolicy` only where physical-zone traversal, membership, and count semantics match exactly. Preserve physical order for observable first-match selection and preserve explicit strategic order for preference selection. DCI/JIT predicates and discard timing remain with strategy owners. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

`PayloadZonePolicy` now accepts any Card range with `begin()` / `end()` semantics, so fixed-size route cost plans can reuse the same physical membership and count traversal without temporary vectors or duplicated STL scans. The issue-1673 Secret Box deadline cost plan is the first `std::array<Card, 3>` consumer. Canonical route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_1673_secret_box_payload_deadline_policy.inc Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163

`PayloadZonePolicy` and `PayloadPreferencePolicy` are final utility classes with static operations only. Keep their responsibilities narrow: physical-zone mechanics belong to the zone policy, strategic payload ordering belongs to the preference policy, and route-specific DCI/JIT admission remains outside both classes.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing labels, opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that owner at the established Engine member boundary. Preserve setup declaration order and hand, Active, Bench, and Prize transitions. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name bridge and intrinsic-classification compatibility seam. Registered `CardDefinition` lookup remains canonical for migrated metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

Reusable exact-print classification should flow through `CardDefinitionPredicates` before adding another raw `definition.kind`, `definition.trainer_kind`, or `definition.pokemon_types` comparison. Compatibility free functions may delegate to that owner until direct consumers migrate. Canonical definition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp

`src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after the Engine caller resolves visibility. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns. K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Keep `KnowledgeCopyPolicy` as a final arithmetic utility class with no Engine or State ownership. New copy-count helpers should compose its arithmetic only after the caller has decided which zones are legally visible in K0 or K1.

## Shared policy owners

Before adding a route-local loop or helper, reuse an existing owner when ordering and semantics match exactly:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and board indices: `src/trace_engine_v2/core/board_state_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.
- Intrinsic exact-print predicates: `src/cards/card_definition.hpp` via `CardDefinitionPredicates`.
- Optional card-effect classifier dispatch: `src/rules/card_context.hpp` via `CardContext::classify`.
- Oricorio connector decision projection: `src/trace_engine_v2/core/routes/oricorio_connector_policy.inc` via `OricorioConnectorPolicy`.
- Legacy Star/Quick Ball decision projection: `src/trace_engine_v2/core/routes/issue_1016_legacy_star_quick_ball_policy.inc` via `LegacyStarQuickBallPolicy`.
- Tate/provenance composition: `src/trace_engine_v2/core/tate/package.inc`; preserve its three included member fragments in their current order until each fragment has its own canonical lower-level owner.
- Battle Compressor / VS Seeker route policy: `src/trace_engine_v2/core/routes/battle_compressor_vs_seeker_policy.inc`.
- Hisuian Heavy Ball route policy: `src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc`.
- Shared search-connector K1 fallback traversal and candidate sets: `src/trace_engine_v2/core/routes/search_connector_helpers.inc` via `SearchFallbackPolicy`.

## Next cleanup steps

1. Keep `composition/steven_blender_overrides.inc` and `core/routes/steven/package.inc` as the canonical Steven organization boundaries. Continue deleting forwarding paths only after repository-wide and source-contract references are proven gone.
2. Keep `core/routes/late_steven_route_policy.inc` as the canonical late-Steven policy body. Migrate audit and traceability anchors to the canonical path, then delete the historical `part_010_late_steven_override.inc` source-contract mirror.
3. Continue retiring other composition-only `part_*steven*` forwarders when a complete function body or macro lifetime can move at the identical textual boundary. The issue-3221 and issue-3222 root semantic forwarders are complete and should not be recreated.
4. Migrate direct `CardContext` bridge consumers to `core/adapters/card_context_adapter.hpp`. Remove the old forwarding include only after references are proven gone. New bridge construction should use `CardContextAdapterCallbacks`.
5. Migrate remaining `LegacyCardCatalog` and intrinsic compatibility rows one card at a time. Reuse `CardDefinitionPredicates` for intrinsic classification before adding new raw metadata checks; delete a row only after explicit `CardDefinition` registration, exact-print source, and focused metadata coverage exist.
6. Continue replacing route-local board traversals with `core/board_state_policy.inc` only when first-match/order semantics are identical. The Quick Ball/Latias completion route is migrated; preserve its route-local JIT, evolution, and connector decisions.
7. Keep Forretress reusable scenario and runtime ownership under `core/forretress/`, with `core/forretress/package.inc` as the namespace-scope composition owner. Remove `part_forretress_ex_combo.inc` only after repository-wide and source-contract consumers are proven migrated.
8. Keep the Tate/provenance package as the sole composition include for its three adjacent legacy fragments. Migrate those fragment bodies into named lower-level owners one at a time, preserving member order and leaving strategy decisions in Engine.
9. Prefer named pure-projection members over route-local anonymous projections when a projection is reused or has a distinct policy contract.
10. Prefer named final stateless policy classes for reusable arithmetic or traversal seams. Do not move route admission, hidden-zone visibility decisions, DCI/JIT timing, or target preference into a utility class merely to reduce line count.
11. Keep route-local final policy classes limited to reusable pure projections. Engine callers continue to own state reads, DCI/JIT visibility, route admission, and physical actions unless a lower card/rules layer owns the printed transition.
12. Route new optional intrinsic card classifiers through `CardContext`'s shared classifier dispatch seam rather than duplicating callback-null checks in each public operation.
13. Reuse generic `PayloadZonePolicy` range operations for fixed-size Card cost plans only when the query is purely physical membership, count, or first-match traversal; keep route-specific DCI/JIT admission outside the utility class.
14. Keep `composition/engine_body.inc` pointed directly at `core/routes/battle_compressor_vs_seeker_policy.inc`; the historical root compatibility include is retired and should not be recreated.
15. Continue moving complete route-specific member bodies out of numbered `part_*.inc` fragments into named `core/routes/` owners only when the exact textual member boundary can be preserved. The next search-connector boundary is the Mysterious Treasure route beginning in `part_009a.inc`; move its continuation only when the full member can migrate atomically without changing declaration order. C++ textual-include semantics: https://eel.is/c++draft/cpp.include Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113

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
