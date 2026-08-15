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
src/trace_engine_v2/core/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Ownership map

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical metadata lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations.
- `src/trace_engine_v2/core/card_catalog.inc` owns legacy names and intrinsic classification fallbacks that have not migrated into registered card definitions.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable Active/Bench traversal, board indices, ranked board queries, and prior-turn evolution timing.
- `src/trace_engine_v2/core/routes/` owns named strategic route policy, including Supporter mode choice and route admission.
- `src/trace_engine_v2/composition/engine_body.inc` and `composition/post_014a_overrides.inc` own textual composition and temporary macro lifetimes.
- Engine strategy retains DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, lock schedules, readiness, payload timing, and action ranking.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` is the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains canonical where a `CardDefinition` exists. `src/trace_engine_v2/core/deck_knowledge.inc` owns K0/K1 deck-knowledge transitions and must remain independent of strategic card selection. Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog work should migrate one exact print at a time into explicit registration, then remove only the matching compatibility row after metadata and parity tests exist.

## Shared policy owners

Reusable rules-neutral queries belong in named shared owners rather than route fragments. `board_state_policy.inc` owns board traversal and timing predicates; Garbodor lock semantics stay in `core/garbodor_lock_policy.inc`; route-specific decisions remain under `core/routes/`. Cleanup plan references must preserve these ownership boundaries so common helpers do not absorb DCI, AMR, lock timing, or action ranking.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse its physical-order and strategic-priority helpers only when the caller has identical ordering semantics. DCI/JIT timing and the decision to spend a payload stay with strategy. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Next payload work should replace ad hoc scans only when their semantics exactly match the canonical payload helpers. Preserve route-specific exclusions and current-turn JIT requirements.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns reusable setup-state lifecycle transitions and observations. Historical continuation fragments may call that owner, while route choice and readiness prioritization remain in strategy. Evolution timing and other raw game rules must continue to use their canonical rules/policy owners and direct sources. Advanced player manual: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next setup-lifecycle work should consolidate duplicated state-transition bookkeeping only when call order and trace timing are unchanged.

## Compatibility seams

`src/trace_engine_v2/part_000.inc` is the legacy card-catalog bridge. `part_001.inc` now reuses that bridge instead of forwarding to the retired `core/card_classification.inc`. Keep the source-contract mirror and historical anchor padding until the corresponding raw-source tests and external references are deliberately migrated. Canonical compatibility owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc

Compatibility `.inc` files may forward to a canonical owner when historical source contracts still require the path. They must not become a second executable implementation. Before retiring a compatibility path, search raw-source tests, repository documentation, issue/PR references, and composition includes.

## Composition cleanup

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, Engine member boundaries, and relative include roots. C++ textual include semantics: https://eel.is/c++draft/cpp.include

When a root `part_*` file contains one complete semantic policy block, move that block under its existing semantic owner and compose it at the same textual boundary. Keep card/rule URLs with the moved implementation. Do not introduce forwarding-only sequencers merely to reduce file length.

Professor Burnet ready-turn admission and resolver mechanics remain owned by `src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc`. Keep its ordered payload/setup-dead priority tables and shared deck-to-discard transition local unless another proven route needs exactly the same semantics. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc

Tate & Liza Supporter consumption plus switch/draw mode resolution now lives in `src/trace_engine_v2/core/routes/tate_liza_policy.inc`; `part_008a.inc` retains the established member boundary and composes that owner directly. Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148 Advanced player manual: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next composition targets should meet all of these conditions:

1. The candidate has one clear semantic owner.
2. Its complete function body or complete macro lifetime can move intact.
3. The include can remain at the identical Engine member boundary.
4. Direct card, rule, ruling, and issue URLs move with the implementation.
5. Focused tests and representative `--simulate-this` traces cover the affected route.

## Card migration workflow

1. Search open issues for an existing migration owner.
2. Avoid parallel migration while another agent owns the card.
3. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata/classification first.
6. Locate the live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused metadata and printed-legality/effect tests.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration exposes incorrect gameplay behavior, use the bug-confirmation workflow rather than folding the behavior fix into cleanup.

## Active card migrations

Do not create a parallel migration while these existing owners remain active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620

## Validation gate

Every cleanup branch must pass:

- strict C++20 compilation and the full test suite;
- sanitizer CI;
- representative `--simulate-this` traces with route-quality review;
- paired T2/T3 setup-probability generation;
- generated-report/source-binding contracts, unless an already-tracked evidence refresh is the only failing contract.

Behavior-preserving cleanup should leave setup probabilities within the repository's deterministic paired-matrix expectations. Any unexpected gameplay delta requires investigation before merge.

## Next work

- Continue retiring one-purpose root `part_*` policy blocks into `core/routes/` only where the complete semantic boundary is clear.
- Continue migrating legacy metadata rows into explicit `CardDefinition` registration one card at a time.
- Replace duplicated board traversals only when their ordering and tie semantics exactly match `board_state_policy.inc`.
- Replace duplicated payload scans only when their physical-order or strategic-priority semantics exactly match the canonical payload helpers.
- Keep raw-source compatibility shims narrow, documented, and free of executable ownership.
