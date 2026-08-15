# Card Class Cleanup

This file is the live architecture and migration plan. Completed cleanup-wave history belongs in Git history so the current plan stays actionable.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not inspect raw `Engine` or `State` data or include trace-engine implementation fragments.

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

## Architecture owners

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts including name, canonical print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box, Pokémon V, ACE SPEC, and Basic Energy classification.
- `src/cards/card_registry.hpp` owns deterministic registration and canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1, setup-axis value, locks, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated compatibility names and intrinsic classification fallbacks.
- `src/trace_engine_v2/composition/engine_body.inc` owns ordered Engine composition and macro lifetimes.
- `src/trace_engine_v2/composition/post_014a_overrides.inc` owns the post-search override composition boundary.

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual include semantics: https://eel.is/c++draft/cpp.include

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata can move first. Move printed resolution only after locating the live resolver and the reusable `CardContext` operations. Keep strategic selection and timing in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify each `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata and classification first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration exposes wrong gameplay, use the normal bug-confirmation workflow rather than hiding a gameplay fix inside cleanup.

## Composition cleanup

The composition spine has already absorbed several forwarding-only stages. Continue by retiring only a root `part_*` shim whose canonical owner can replace it at the identical textual boundary. Never recreate a forwarding-only sequencer after removing one.

The next explicit composition candidate remains the Professor Burnet compatibility shim. `src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc` owns Burnet ready-turn admission and resolution. `part_011_burnet_thinning_override.inc` may be deleted only when `post_014a_overrides.inc` directly includes the canonical owner under the same `play_professor_burnet` alias lifetime. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc

After that seam is retired, inspect another one-purpose root fragment only when its complete macro lifetime or function body can move intact. Route admission and projection policy stays under `src/trace_engine_v2/core/routes/`.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

- `PayloadZonePolicy::first_iterator_matching()` owns first-match physical-zone traversal.
- `PayloadZonePolicy::contains_matching()` owns predicate-based zone membership.
- `PayloadZonePolicy::count_matching()` owns predicate-based zone cardinality.
- `PayloadZonePolicy::first()` preserves physical order when selection order is observable.
- `PayloadZonePolicy::contains()` and `count()` own generic payload membership and count semantics.
- `PayloadZonePolicy::contains_card()` owns concrete-card membership.
- `PayloadPreferencePolicy` owns the explicit strategic payload preference order.

The Earthen Vessel VSTAR-window route now delegates held-Dragon membership to `payload_zone_contains(state_.hand)` rather than maintaining another `std::any_of(..., is_payload)` scan. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 Canonical payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Next payload step: replace remaining ad hoc Dragon-payload scans only where semantics exactly match a canonical payload query. Preserve physical-order selection where order matters. Keep DCI/JIT predicates and discard timing at strategy owners.

## Knowledge cleanup

`src/trace_engine_v2/core/deck_knowledge.inc` owns fixed-list and public-zone copy arithmetic after the Engine caller has resolved visibility.

`searchable_copy_count(Card)` now owns the existing visibility-aware count used by routes that need the number of copies physically searchable from the remaining deck. It uses exact deck state after a legal deck inspection or complete Prize inspection and fixed-list/public-zone arithmetic while hidden deck-versus-Prize placement is unresolved. Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle Advanced search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The Earthen Vessel VSTAR-window route now consumes that named seam rather than re-encoding the same K0/K1 branch locally. Next knowledge step: migrate repeated copy arithmetic only when the caller has exactly the same visibility contract. Hidden-zone visibility, Prize deduction, target preference, DCI/UDP/AMR, and route admission remain Engine strategy concerns.

## Board and Forretress cleanup

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex`, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery, deterministic ranked board queries, and prior-turn evolution timing.

`src/trace_engine_v2/core/forretress/contract.inc` owns Pineco, Forretress ex, Pineco-to-Forretress, and Regidrago-line board-role classification. Runtime and printed Exploding Energy resolution remain under `src/trace_engine_v2/core/forretress/`.

Next Forretress step: replace only board-object identity or prior-turn evolution checks that exactly match existing board-policy classifiers. Preserve exact state-count queries, entry-turn timing, route ordering, attachment distribution, retreat planning, and strategic ranking. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Setup and turn lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup labels, opening-deck initialization, mulligans, Prize dealing, and setup traces. `SetupRecipePolicy` owns recipe-presence and exact-count predicates.

`src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn reset composition. `TurnActionStatePolicy::reset()` clears generic action flags and same-turn discard tracking. `TransientTurnLockPolicy::reset()` owns scenario-scoped one-turn lock reset.

Next setup step: move state-transition helpers from opening Active/Bench setup only after exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Next turn step: centralize only exact duplicate reset bundles while preserving their order before the mandatory turn draw. Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Shared policy owners

Before adding another route-local helper or loop, inspect these owners first:

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`
- Deck knowledge and copy arithmetic: `src/trace_engine_v2/core/deck_knowledge.inc`
- Board traversal and board identity: `src/trace_engine_v2/core/board_state_policy.inc`
- Garbodor and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`
- Setup lifecycle: `src/trace_engine_v2/core/setup_lifecycle.inc`
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`

Reuse a named seam only when ordering and semantics match exactly. A superficially similar route with different visibility, DCI, timing, or preference semantics needs its own strategy logic.

## Validation gate

A cleanup PR is mergeable only when all of the following hold:

1. Release compilation succeeds.
2. Focused tests and the full regression suite show no new failure.
3. Sanitizer and structural checks show no new failure.
4. Representative `--simulate-this` traces preserve legal action ordering and readiness.
5. The paired T2/T3 matrix shows no unexplained drift.
6. The PR contains no gameplay behavior change.

Known baseline failures must remain tied to their existing issue. Any newly discovered gameplay defect uses the separate bug-confirmation workflow.