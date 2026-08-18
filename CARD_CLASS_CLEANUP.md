# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup details remain in Git history. Keep this file focused on current ownership, remaining boundaries, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data. Route policy may consume reusable card and rules interfaces, while card modules must remain independent of route-specific DCI, UDP, AMR, K0/K1, lock, and readiness decisions.

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

## Current ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts and reusable intrinsic classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_definition.hpp
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations and the generic resolving-Trainer source lifecycle. Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` is the trace-engine bridge for reusable card effects.
- `src/trace_engine_v2/core/card_catalog.inc` owns the shrinking unmigrated name and classification compatibility layer. Registered `CardDefinition` lookup stays the preferred metadata path.
- `src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered composition spine: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
- `src/trace_engine_v2/core/turn_lifecycle.inc` owns turn-number assignment, action-state reset, transient lock reset, start-of-turn draw, and first-turn restriction tracing. Its reset orchestration has one `TurnLifecyclePolicy` owner.
- `src/trace_engine_v2/core/deck_knowledge.inc` owns copy arithmetic after visibility is resolved. K0/K1 visibility decisions stay in Engine strategy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
- `src/trace_engine_v2/core/payload_hand_policy.inc` owns reusable Dragon-payload zone and preference traversal.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable board traversal and board-index queries.
- `src/trace_engine_v2/core/routes/search_connector_helpers.inc` owns complete K1 fallback selectors for Mysterious Treasure, Quick Ball, and Ultra Ball.
- `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc` remains the live strategic target-priority owner composed by `part_009a.inc`. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- `src/trace_engine_v2/core/locks/garbodor_policy.inc` remains the sole Garbodor scenario-timing and Ability-lock policy owner. Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57

2026-08-18 cleanup checkpoint: `src/trace_engine_v2/core/regidrago_line_helpers.inc` remains a compatibility include at the post-search Arven boundary, but it no longer contains a fallback include to the retired `core/card_classification.inc` path. The canonical catalog is composed first and owns `is_regidrago_v_line()`. Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136

2026-08-18 search-connector checkpoint: `src/trace_engine_v2/core/routes/search_connector_helpers.inc` now gives post-search outlet feasibility explicit owners for card presence, survival of the current search discard, and Ultra Ball payability. Keep future K1 connector cleanup on these named helpers so route callers do not recreate hand-count and discard-cost arithmetic. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146 Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163

## Active card migrations

No open migration issue is assumed by this plan. Before starting a card migration, search the current issue tracker and branch set for an existing owner. A migration should move intrinsic metadata and classification before printed resolution, then move printed resolution only after its live resolver and reusable `CardContext` operations are identified.

Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, lock policy, and setup-readiness policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

`src/trace_engine_v2/composition/opening_engine_overrides.inc` owns the early override chain, including the live `part_009a.inc` Mysterious Treasure boundary. Do not infer that a root `.inc` is dead from naming or code-search absence alone. Prove its composition reachability from the ordered include spine before retirement.

`src/trace_engine_v2/composition/post_014a_overrides.inc` owns the later Arven continuation that consumes the Regidrago-line compatibility seam. The shim now asserts that canonical classification was already composed instead of trying to reopen the retired classifier path.

`src/trace_engine_v2/composition/steven_blender_overrides.inc` owns the Steven/Brilliant Blender macro-composition boundary. `src/trace_engine_v2/core/routes/steven/package.inc` is the canonical organized Steven route package. Retire remaining forwarders only when source-contract references and macro lifetime are both proven migrated.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse `PayloadZonePolicy` only where physical-zone traversal, membership, and count semantics match exactly. Preserve physical order for observable first-match selection and preserve explicit strategic order for preference selection.

`PayloadZonePolicy` and `PayloadPreferencePolicy` remain final utility classes with static operations. Physical-zone mechanics belong to the zone policy, strategic payload ordering belongs to the preference policy, and route-specific DCI/JIT admission remains outside both. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

Mysterious Treasure keeps strategic Dragon/Psychic target preference in `core/mysterious_treasure_target_policy.inc`; post-search K1 fallback legality and order stay in `core/routes/search_connector_helpers.inc`. Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup/policies.inc` owns pure setup recipe predicates, setup constants, and scenario labels. `src/trace_engine_v2/core/setup_lifecycle.inc` owns physical opening-deck initialization, mulligan handling, Prize dealing, and setup trace emission. `src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn reset and start-of-turn mechanics.

Keep setup and turn mechanics source-linked to the advanced manual and official rules. Advanced manual: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name and intrinsic-classification compatibility seam. New exact-print classification should flow through registered `CardDefinition` data before another legacy classifier is introduced. Canonical registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

`src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after the Engine caller resolves visibility. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns. K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Keep `KnowledgeCopyPolicy` as a final arithmetic utility with no Engine or State ownership. New copy-count helpers may compose its arithmetic only after the caller has decided which zones are legally visible.

## Shared policy owners

Before adding a route-local loop or helper, reuse an existing owner when ordering and semantics match exactly:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and indices: `src/trace_engine_v2/core/board_state_policy.inc`.
- K0/K1 copy arithmetic: `src/trace_engine_v2/core/deck_knowledge.inc`.
- Search connector fallback order: `src/trace_engine_v2/core/routes/search_connector_helpers.inc`.
- Mysterious Treasure strategic target order: `src/trace_engine_v2/core/mysterious_treasure_target_policy.inc`.
- Turn reset mechanics: `src/trace_engine_v2/core/turn_lifecycle.inc`.
- Scenario extension traversal: `src/trace_engine_v2/core/scenario_extension_policy.inc`.
- Garbodor lock behavior: `src/trace_engine_v2/core/locks/garbodor_policy.inc`.
- Card effect bridge: `src/trace_engine_v2/core/adapters/card_context_adapter.hpp`.

## Next cleanup steps

1. Continue deleting forwarding `.inc` files only after tracing them from `composition/engine_body.inc` and proving they are absent from the live include graph. A missing historical target is evidence of stale code, while composition reachability is the decisive check.
2. Keep `core/mysterious_treasure_target_policy.inc` until `part_009a.inc` is migrated to a canonical organized route package. Preserve its target order and direct Mysterious Treasure card-data citation during that move.
3. Continue moving complete route bodies from numbered `part_*` fragments into `core/routes/` packages at identical textual boundaries, with macro lifetime documented at the composition owner.
4. Keep card metadata and printed-effect migrations flowing through `src/cards/` and `src/rules/`; do not move DCI, UDP, AMR, connector domination, K0/K1, or opponent-pressure policy into card classes.
5. Prefer one named policy owner for each reusable traversal or state-reset operation. Remove micro-forwarders after consumers use that owner directly.
6. Preserve all source URLs beside rules-sensitive code during moves. Use the advanced manual for procedural rules and exact card records for printed effects.

## One-card workflow

1. Search open issues and active branches for an existing migration owner.
2. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
3. Add or extend one primary card module and register it explicitly.
4. Preserve the exact external print identifier and direct card-data URL.
5. Route reusable physical operations through `CardContext` rather than raw Engine state.
6. Leave strategic target choice, DCI/UDP/AMR, K0/K1, locks, and readiness in Engine.
7. Add focused tests for metadata and printed legality/effect boundaries.
8. Run strict CI, representative `--simulate-this` traces, sanitizers, and the paired T2/T3 matrix before merge.

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow and keep the behavior fix out of cleanup.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer and structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering and readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to an existing issue and remain unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow.
