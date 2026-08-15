# Card Class Cleanup

This file is the live architecture and migration plan. Historical cleanup-wave notes remain in Git history. Keep this document limited to current ownership, remaining work, and validation requirements.

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

`src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.

`src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and direct source URL.

`src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup. Registry source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

`src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.

Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata/classification can move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical Engine composition owner. `composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation. `composition/post_014a_overrides.inc` owns late-search composition.

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission/projection/decision policy stays under `src/trace_engine_v2/core/routes/`. Retire a compatibility forwarder only after its parent is retargeted at the identical textual boundary and raw-source readers or anchors are migrated. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The numbered policy fragments `part_013.inc`, `part_014a.inc`, and `part_014b.inc` are retired. `composition/opening_engine_overrides.inc` composes `core/supporter_legacy_runtime.inc` directly; `composition/engine_body.inc` composes `turn_action_policy_runtime.inc` directly; `composition/post_014a_overrides.inc` composes `core/recovery_supporter_policy.inc` directly at the historical `choose_supporter_original` macro boundary. Recovery owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/recovery_supporter_policy.inc

Root `part_000.inc` and `part_001.inc` remain source-contract shims while unified-test generation, raw-source payload contracts, and same-repository anchors depend on their historical paths. Canonical owners: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc and https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

`PayloadPreferencePolicy::first_in_zone()` preserves physical zone order for callers whose historical behavior depended on the first matching card in that zone. `PayloadPreferencePolicy::first_preferred()` preserves the explicit strategic preference order used for hand and known-deck selection. Zone-wide predicates use `contains_in_zone()` and `count_in_zone()` so membership/count semantics are distinguishable from preference-order selection at the call site.

Completed in `cleanup-1786767958859`: renamed the preference-order helper to `first_preferred()` and clarified zone predicates as `contains_in_zone()` and `count_in_zone()`. These are behavior-preserving ownership changes. DCI/JIT admission and payload route policy remain at their existing Engine callers. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Next payload step: inventory remaining ad hoc Dragon-payload scans and replace only callers whose established ordering exactly matches `first_in_zone()` or `first_preferred()`. Preserve physical-order callers when order is observable. Do not replace DCI/JIT predicates with generic payload membership.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations. `src/trace_engine_v2/core/forretress/runtime.inc` owns runtime definitions. The split remains necessary while Engine is a textual class body and runtime definitions are emitted after Engine closes.

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery through `board_index_for_card()`, deterministic ranked board queries, and the prior-turn evolution timing predicate. `board_index_best_matching()` preserves first-equal ordering by replacing the selected index only when a later candidate ranks strictly higher, while `board_index_is_benched()` and `best_benched_pokemon_index()` centralize Bench-only ranking. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc C++ `max_element` ordering: https://eel.is/c++draft/alg.sorting#alg.max.min

Completed through `cleanup-1786767670470`: Forretress ex source discovery, Active-role checks, board index conversion, destination lookup, and shared traversal use the canonical board-state vocabulary. Attachment selection, self-Knock-Out, promotion, retreat payment, DCI/UDP/AMR, K0/K1, readiness, and route policy remain at their prior owners.

Completed in `cleanup-1786769773419`: named the Bench board-index role and added deterministic ranked board selection over the existing Active-first visitor. The new seam retains first-equal ordering and does not move the Forretress strategy comparator or change card legality, promotion, retreat payment, attachment distribution, self-Knock-Out, DCI/UDP/AMR, K0/K1, readiness, or route policy.

Next mechanical Forretress step: route the existing post-Ability retreat-target comparator through `best_benched_pokemon_index()` only after focused coverage demonstrates identical target selection for strict rank wins and equal-rank ties. Preserve the comparator itself and all gameplay policy at the Forretress owner. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117 Core evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

## Steven route cleanup

Named Steven route owners live under `src/trace_engine_v2/core/routes/`. The issue-3653 free-slot connector and issue-3202 VSTAR/Vessel route are composed directly from their canonical owners. The issue-3203 Active-VSTAR route is composed at its established nested parent boundary. Historical forwarders for those routes are retired.

Next Steven step: inventory remaining Steven-named root `part_*.inc` files and retire only composition-only forwarders whose canonical route owner can be included at the exact same member and macro boundary. Preserve route admission, DCI/UDP/AMR, Supporter contention, connector domination, and source URLs. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels together with opening-hand, mulligan, Prize-deal, and setup-trace mechanics. `src/trace_engine_v2/part_005.inc` now composes that canonical owner directly at the established Engine member boundary.

Completed in `cleanup-1786770665212`: removed the redundant `core/simulation_labels.inc` composition edge from `part_005.inc` and retired the empty compatibility shell after confirming the canonical setup lifecycle was already included at the same textual boundary. This is a structure-only cleanup; setup choice policy, K0/K1 state, mulligan counting, Active/Bench selection, Prize placement, declaration order, DCI/UDP/AMR behavior, Supporter contention, and connector domination are unchanged. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/setup_lifecycle.inc C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Next setup step: inventory setup-only helpers still living in `part_005.inc` and move only self-contained setup mechanics into `core/setup_lifecycle.inc` after focused source-contract coverage proves declaration-order and trace equivalence. Keep opening Active/Bench strategy in Engine unless a seam cleanly separates policy choice from setup state transition. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario/timing: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow.
