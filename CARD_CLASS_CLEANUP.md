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

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, resolving-source movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Canonical owners

- Card identity and intrinsic metadata: `src/cards/card_id.hpp`, `src/cards/card_definition.hpp`, and `src/cards/card_registry.hpp`.
- Reusable printed-rules operations: `src/rules/card_context.hpp`.
- Trace-engine card bridge: `src/trace_engine_v2/core/adapters/card_context_adapter.hpp`.
- Ordered Engine composition: `src/trace_engine_v2/composition/engine_body.inc`. Preserve `#define` / `#include` / `#undef` order and member boundaries. C++ textual-include semantics: https://eel.is/c++draft/cpp.include
- Dragon payload traversal and preference: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal: `src/trace_engine_v2/core/board_state_policy.inc`.
- Setup policy and physical setup transitions: `src/trace_engine_v2/core/setup/policies.inc` and `src/trace_engine_v2/core/setup_lifecycle.inc`. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- Knowledge copy arithmetic after visibility is resolved: `src/trace_engine_v2/core/deck_knowledge.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/locks/garbodor_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Search-connector fallback orders: `src/trace_engine_v2/core/routes/search_connector_helpers.inc`.
- Mysterious Treasure strategic target priority: `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc`. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Steven/Brilliant Blender composition boundary: `src/trace_engine_v2/composition/steven_blender_overrides.inc`.
- Source-bounded Steven package: `src/trace_engine_v2/core/routes/steven/package.inc`. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Late-Steven route policy: `src/trace_engine_v2/core/routes/late_steven_route_policy.inc`. Its former `src/trace_engine_v2/part_010_late_steven_override.inc` source-contract mirror is retired after repository-wide reference checks reached zero. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven_route_policy.inc
- Issue-1199 Steven package policy: `src/trace_engine_v2/core/routes/issue_1199_steven_package_policy.inc`; the root compatibility include remains only while source-contract references migrate.
- Forretress namespace and concrete scenario extension: `src/trace_engine_v2/core/forretress/package.inc`; reusable range mechanics stay in `src/trace_engine_v2/core/scenario_extension_policy.inc`.
- Tate composition: `src/trace_engine_v2/core/tate/package.inc`; its named provenance, attachment, and action owners live under `core/tate/`.
- Professor Burnet selection constants: `src/trace_engine_v2/core/routes/professor_burnet/selection_policy.inc`; route admission and physical execution remain in `professor_burnet_ready_turn_policy.inc`. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26

Engine strategy retains route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, lock schedules, readiness, and payload timing. These concerns must not migrate into generic card or traversal utilities merely to reduce line count.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` remains the canonical Dragon-payload zone and preference owner. Reuse generic range operations only for physical membership, count, first-match traversal, or the established payload preference order. DCI/JIT admission and current-turn discard timing stay in Engine. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup/policies.inc` owns pure setup constants and classification, while `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-hand, mulligan, Prize-deal, and setup-trace transitions. Preserve the established `part_005.inc` member boundary until its remaining source-contract anchors migrate. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns unmigrated catalog fallback while registered `CardDefinition` metadata remains canonical. `src/trace_engine_v2/core/deck_knowledge.inc` owns copy arithmetic only after visibility is legally resolved; K0/K1 admission remains Engine-owned. Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

## Shared policy owners

Before adding route-local traversal, reuse the canonical payload, board, setup, knowledge, Garbodor, search-connector, Forretress, Tate, and scenario-extension owners listed above when ordering and semantics match exactly. Route admission, DCI/JIT, hidden-zone visibility, Supporter contention, connector choice, and physical action execution remain with their established strategy, card, or rules layer.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Remaining cleanup steps

1. Keep `composition/steven_blender_overrides.inc` and `core/routes/steven/package.inc` as the canonical Steven organization boundaries. The retired `core/routes/steven_package_policy.inc` and `part_010_late_steven_override.inc` paths must not be recreated.
2. Migrate remaining source-contract references from `part_issue_1199_steven_package_override.inc` to `core/routes/issue_1199_steven_package_policy.inc`, then retire the root compatibility include when repository-wide references reach zero.
3. Retire other composition-only `part_*steven*` forwarding paths only when the complete function body or macro lifetime can move at the identical textual boundary.
4. Keep direct `CardContext` bridge consumers on `core/adapters/card_context_adapter.hpp`; do not recreate the former root forwarding include.
5. Migrate remaining `LegacyCardCatalog` rows one card at a time. Require explicit `CardDefinition` registration, an exact-print source, and focused metadata coverage before deleting a fallback row.
6. Reuse `core/board_state_policy.inc`, `PayloadZonePolicy`, and `PayloadPreferencePolicy` only when traversal and ordering semantics match exactly. Keep route-specific DCI/JIT admission and target selection in Engine.
7. Keep `core/forretress/package.inc` as the sole Forretress namespace-scope composition owner. Split the oversized runtime only when printed Exploding Energy transitions, connector orchestration, and pure projections can move with source-contract anchors and include order preserved.
8. Migrate the remaining `part_014c.inc` consumer to the canonical Forretress package before deleting `part_forretress_ex_combo.inc`.
9. Migrate source-contract anchors for the remaining Tate provenance and attachment mirrors, then retire those mirrors after repository-wide references reach zero.
10. Keep played-Trainer source movement on `CardContext::ResolvingSourceCallbacks` and the centralized resolving-source policy. Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

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
