# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup-wave notes remain in Git history. Keep this document focused on current ownership, remaining work, and validation requirements.

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
src/trace_engine_v2/core/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball is the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts and direct source URLs.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility name bridge for cards without a registered `CardDefinition`.

`LegacyCardCatalog::find()` is the single legacy-table lookup seam. Registered metadata remains the canonical name path, while remaining legacy entries are retired one card at a time after registration and focused metadata coverage. C++20 lookup semantics: https://eel.is/c++draft/alg.find

Next catalog step: migrate one remaining `LegacyCardCatalog` entry only when that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategic policy at their current owners during metadata-only migrations.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata/classification can move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection and simulator policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical Engine composition owner. The named opening, banked-Tapu, lock-removal, late-execution, and late-registry/reporting stages preserve historical textual boundaries and macro lifetimes. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission/projection/decision policy stays under `src/trace_engine_v2/core/routes/`. Retire a compatibility forwarder only after its parent is retargeted at the identical textual boundary and raw-source readers or anchors are migrated. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Root `part_000.inc` and `part_001.inc` remain source-contract shims while unified-test generation, raw-source payload contracts, and same-repository anchors depend on their historical paths.

Next composition step: inventory the remaining named composition stages for a forwarding-only seam whose parent can be retargeted at the identical textual boundary. Retain `opening_legacy_stage.inc` while it still owns live alias setup.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

- `PayloadZonePolicy` owns physical-zone membership, count, and first-match traversal.
- `PayloadPreferencePolicy::first_preferred()` owns the explicit strategic priority order.
- `PayloadPreferencePolicy::contains_card()` owns exact-card membership used by preference scans.
- `PayloadPreferencePolicy::first_preferred_with_positive_count()` adapts count-backed zones without duplicating preference traversal.

These helpers preserve physical order where observable and preserve the existing five-card strategic preference order. DCI/JIT admission, discard timing, K0/K1 timing, and route decisions remain at their strategy owners. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Next payload step: migrate only remaining ad hoc payload scans whose ordering semantics exactly match one of these named seams.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations. `runtime.inc` owns runtime composition and route-facing definitions. `exploding_energy_runtime.inc` owns the contiguous printed Exploding Energy resolver and immediate post-KO mechanics.

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, board-index vocabulary, deterministic ranked board queries, and prior-turn evolution timing. Attachment selection, self-Knock-Out strategy, promotion ranking, retreat planning, DCI/UDP/AMR, K0/K1, readiness, and route priority stay at their current policy owners. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Core evolution rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next Forretress step: extract another exact semantic boundary from `runtime.inc` only when declaration order and route ordering can be preserved exactly.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels, opening-hand mechanics, mulligans, Prize deal, and setup trace mechanics. Opening Active/Bench strategic predicates remain in Engine.

Next setup step: move only state-transition helpers once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario/timing: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam only when ordering and semantics match exactly.

## Current cleanup checkpoint

`cleanup-1786787804740` centralizes two repeated lookup shapes without changing gameplay: `LegacyCardCatalog::find()` owns legacy name-table lookup, and `PayloadPreferencePolicy::contains_card()` owns exact-card membership for strategic payload preference. This reduces ad hoc scans while preserving registry priority, payload priority, K0/K1, DCI/UDP/AMR, Supporter contention, connector domination, and route behavior.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
