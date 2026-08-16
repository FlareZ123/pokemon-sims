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

Keep the Quick Ball reference seam intact while card ownership continues to migrate:

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

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts. Strategy roles such as DCI, UDP, AMR, route priority, and JIT timing remain outside `CardDefinition`.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns the named trace-engine callback bridge.
- `src/trace_engine_v2/core/card_context_adapter.hpp` remains a compatibility include until direct consumers are proven gone.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 state, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` remains the compatibility owner for unmigrated intrinsic metadata and names.

## Current cleanup progress

Card-class metadata tests now share `tests/support/card_registry_test_utils.hpp` for the repeated `find_definition()` null check and assertion primitive. Appletun and Arven are the first consumers. Keep exact card/rule source URLs beside metadata or rule-sensitive assertions in those tests. This helper is test-only and must not become a second gameplay registry or strategy layer.

Next test-support step: migrate other card-class metadata tests only when their assertion semantics match this helper exactly. Keep card-specific effect setup, route policy, and gameplay state builders local to the focused test that owns them.

Next adapter step: construct future trace-engine card bridges with `CardContextAdapterCallbacks` at the canonical `core/adapters/card_context_adapter.hpp` owner. Migrate direct consumers of the forwarding `core/card_context_adapter.hpp` include when their seams are touched, then remove that forwarding include only after repository-wide references are proven gone.

Next catalog step: migrate remaining `LegacyCardCatalog` entries one card at a time. Delete a compatibility row only after the card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata coverage.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620

For each migration, metadata/classification may move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify each `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata/classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces when the workflow calls for them, and the paired T2/T3 matrix before merge.

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow instead of combining the fix with cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Named semantic owners already exist under `core/routes/`, `core/forretress/`, and the composition directory. Prefer rewiring a live composition boundary directly to an existing canonical owner before introducing another forwarding `part_*` include. Preserve macro lifetime and member declaration order exactly.

Next composition step: migrate a root compatibility seam only when its complete function body or macro lifetime can move intact to the canonical semantic owner. Keep tooling-only compatibility paths where source-contract or unified-test generation still reads them directly.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` remains the canonical Dragon-payload query owner. Reuse its physical-order and explicit-preference helpers only when the caller's selection semantics match exactly. Keep DCI/JIT predicates and discard timing at strategy owners. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-deck initialization, mulligans, Prize dealing, setup labels, and setup trace mechanics. Keep setup recipe predicates centralized without moving strategic route decisions into the lifecycle layer. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy metadata/name compatibility bridge. `src/trace_engine_v2/core/deck_knowledge.inc` owns reusable public-copy arithmetic after Engine callers resolve K0/K1 visibility. Hidden-zone visibility, Prize deduction, route admission, and DCI/UDP/AMR remain strategy concerns. Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

## Shared policy owners

Before adding another route-local loop or helper, check these owners and reuse them only when ordering and semantics match exactly:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal and board-index vocabulary: `src/trace_engine_v2/core/board_state_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`.
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.
- Deck-knowledge copy arithmetic: `src/trace_engine_v2/core/deck_knowledge.inc`.

Physical-order selectors must remain physical-order selectors. Explicit strategic preference must remain explicit. Hidden-information visibility must be resolved before generic count helpers are called. Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative traces preserve legal action ordering/readiness where applicable, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to an existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow.
