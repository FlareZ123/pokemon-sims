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
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Core ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` and `src/cards/card_registry.hpp` own intrinsic exact-print metadata, reusable intrinsic classification, deterministic registration, and canonical lookup. Canonical owners: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations and the generic resolving-Trainer source lifecycle. Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` is the sole trace-engine bridge for reusable card effects.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 visibility, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/composition/engine_body.inc` owns ordered Engine composition. Mechanical `.inc` cleanup must preserve macro lifetime, include order, declaration order, member boundaries, and relative roots. C++ textual include semantics: https://eel.is/c++draft/cpp.include

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` owns reusable Dragon payload membership and established payload-preference traversal. Reuse it for physical membership, count, and first-match projections only when ordering semantics match exactly. DCI/JIT admission, discard timing, and route-specific target preference remain in Engine or the route owner. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup/policies.inc` owns pure recipe/config projection and setup constants. `src/trace_engine_v2/core/setup_lifecycle.inc` owns physical opening-hand, mulligan, Prize, and setup-trace transitions. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy metadata/classification compatibility seam. Registered `CardDefinition` lookup remains canonical. `src/trace_engine_v2/core/deck_knowledge.inc` owns copy arithmetic only after Engine has resolved legal visibility. K0/K1 visibility, Prize deduction, target preference, DCI/UDP/AMR, and route admission remain strategy concerns. Knowledge-state contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

## Shared policy owners

Reuse an existing policy only when ordering and semantics match exactly.

- Dragon payload membership and preference: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and board indices: `src/trace_engine_v2/core/board_state_policy.inc`.
- K1 copy arithmetic after visibility is resolved: `src/trace_engine_v2/core/deck_knowledge.inc`.
- Setup recipe/config projection: `src/trace_engine_v2/core/setup/policies.inc`; physical setup transitions: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/locks/garbodor_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Search-connector K1 fallback orders: `src/trace_engine_v2/core/routes/search_connector_helpers.inc` via `SearchConnectorFallbackPolicy`.
- Mysterious Treasure strategic target priority: `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc`. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Tapu Lele-GX copy-aware connector projection: `src/trace_engine_v2/core/tapu_connector_policy.inc`; board presence and legal K0/K1 copy knowledge stay in Engine. Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
- Oricorio connector projection: `src/trace_engine_v2/core/routes/oricorio_connector_policy.inc`; live Ability availability, visibility, Bench space, ordering, and actions stay in Engine. Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
- Legacy Star/Quick Ball pure projection: `src/trace_engine_v2/core/routes/issue_1016_legacy_star_quick_ball_policy.inc`; DCI/JIT and projection-only state changes stay in Engine. Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Professor Burnet selection constants: `src/trace_engine_v2/core/routes/professor_burnet/selection_policy.inc`; route admission, Supporter contention, Energy projection, and physical actions stay in `professor_burnet_ready_turn_policy.inc`. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Scenario extension traversal and extension-first lookup: `src/trace_engine_v2/core/scenario_extension_policy.inc`; concrete Forretress/Garbodor ownership remains in `src/trace_engine_v2/core/forretress/package.inc`.

## Steven organization

`src/trace_engine_v2/composition/steven_blender_overrides.inc` is the canonical Steven/Brilliant Blender macro-composition boundary. The source-bounded Steven route family remains organized under `src/trace_engine_v2/core/routes/steven/package.inc`.

The late-Steven route now has a dedicated package at `src/trace_engine_v2/core/routes/late_steven/package.inc`. The package composes:

```text
late_steven/admission_policy.inc
late_steven_route_policy.inc
```

`LateStevenAdmissionPolicy` is a final stateless pure-projection class. It may combine already-resolved Item, Supporter, and Latias Ability permissions. Engine remains responsible for resolving live lock state through `item_locked()`, `supporter_allowed()`, and `ability_available_for_pokemon()`, plus K0/K1 visibility, route admission, DCI/JIT, target preference, and physical actions. This separation prevents a scenario label from becoming a second source of truth for mutable lock state. Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148 Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125 Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76 Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The historical `src/trace_engine_v2/part_010_late_steven_override.inc` remains only as a source-contract mirror until repository-wide audit and traceability anchors are migrated. Do not restore it to composition.

The issue-1199 Steven package remains at `src/trace_engine_v2/core/routes/issue_1199_steven_package_policy.inc`. K0/K1 admission, DCI/JIT, Supporter contention, route priority, and physical actions stay with the route owner. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Remaining cleanup

1. Migrate audit and traceability anchors from `part_010_late_steven_override.inc` to `core/routes/late_steven/package.inc` or the exact owned child, then retire the historical mirror when repository-wide references are gone.
2. Continue retiring composition-only `part_*steven*` forwarders only when complete function bodies or macro lifetimes can move at identical textual boundaries.
3. Keep direct `CardContext` bridge consumers on `core/adapters/card_context_adapter.hpp`; do not recreate the retired root forwarding include.
4. Migrate remaining `LegacyCardCatalog` rows one card at a time. Require explicit `CardDefinition` registration, exact-print source, and focused metadata coverage before deleting a compatibility row.
5. Continue replacing route-local board traversals with `core/board_state_policy.inc` only when first-match and ordering semantics are identical.
6. Keep `core/forretress/package.inc` as the namespace-scope Forretress owner and `core/scenario_extension_policy.inc` as the reusable range/lookup owner. Split `core/forretress/runtime.inc` only at source-contract-safe boundaries.
7. Keep `core/tate/package.inc` as the Tate/provenance composition owner while remaining historical source-contract mirrors migrate.
8. Prefer named final stateless policy classes for reusable pure projections. Keep route admission, hidden-zone visibility decisions, DCI/JIT timing, target preference, and physical state transitions in their established owners.
9. Keep played-Trainer source movement on `CardContext::ResolvingSourceCallbacks`; future Trainer migrations must preserve the hand -> resolving -> discard lifecycle. Advanced Item/Trainer procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

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
