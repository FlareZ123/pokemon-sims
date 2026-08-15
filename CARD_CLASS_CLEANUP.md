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

`src/trace_engine_v2/core/card_catalog.inc` owns the compatibility name bridge for cards that have not yet migrated into `CardDefinition`. Completed in `cleanup-1786780798661`: registered cards were removed from the unreachable legacy fallback, and the remaining compatibility labels were grouped under `LegacyCardCatalog` instead of a mixed switch containing both registered and legacy cards. Registry lookup remains the first and canonical name path, so this is behavior-preserving structure work. Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

Next catalog step: migrate the remaining `LegacyCardCatalog` entries one card at a time through the normal ownership workflow. Delete each compatibility row only when that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategic policy at their current owners during metadata-only migrations.

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical Engine composition owner. `composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation. `composition/post_014a_overrides.inc` owns late-search composition. `composition/opening_legacy_stage.inc` owns the historical `part_003.inc` opening edge and delegates its continuation to `composition/opening_state_completion_stage.inc`, which owns the Garbodor ability alias, `part_004.inc` / `part_005.inc`, and the `might_be_unseen` teardown. `composition/middle_policy_stage.inc` sequences the named banked-Tapu and lock-removal substages. `composition/banked_tapu_policy_stage.inc` owns the banked-Tapu alias, search-item runtime, and route-policy boundary. `composition/lock_removal_policy_stage.inc` owns the shared lock-removal include and the `post_014a_overrides.inc` alias boundary. `composition/late_legacy_stage.inc` sequences `composition/late_execution_stage.inc` and `composition/late_registry_reporting_stage.inc`; those substages own the `part_014c.inc` execution aliases and the `part_015.inc` / `part_016.inc` registry-reporting aliases respectively.

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission/projection/decision policy stays under `src/trace_engine_v2/core/routes/`. Retire a compatibility forwarder only after its parent is retargeted at the identical textual boundary and raw-source readers or anchors are migrated. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The numbered policy fragments `part_013.inc`, `part_014a.inc`, and `part_014b.inc` are retired. `composition/opening_engine_overrides.inc` composes `core/supporter_legacy_runtime.inc` directly; the named middle banked-Tapu substage composes `turn_action_policy_runtime.inc` directly; `composition/post_014a_overrides.inc` composes `core/recovery_supporter_policy.inc` directly at the historical `choose_supporter_original` macro boundary. Recovery owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/recovery_supporter_policy.inc

Root `part_000.inc` and `part_001.inc` remain source-contract shims while unified-test generation, raw-source payload contracts, and same-repository anchors depend on their historical paths. Canonical owners: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc and https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc

Completed in `cleanup-1786780558195`: extracted the banked-Tapu replacement and lock-removal wiring from `composition/engine_body.inc` into `composition/middle_policy_stage.inc`. The extraction preserves the exact `opening_engine_overrides.inc` -> `turn_action_policy_runtime.inc` -> banked-Tapu policy -> lock-removal policy -> `post_014a_overrides.inc` order and keeps the temporary-alias entry/exit assertions beside the code they guard. `engine_body.inc` now composes named opening, middle, and late stages at the same class-member boundaries. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Completed in `cleanup-1786782565070`: split the middle composition owner into named banked-Tapu and lock-removal substages. The banked-Tapu substage preserves `opening_engine_overrides.inc` -> `turn_action_policy_runtime.inc` -> `core/routes/banked_tapu_retreat_policy.inc` together with the retreat-alias checks. The lock-removal substage preserves `core/forest_field_blower_policy.inc` -> alias setup -> `post_014a_overrides.inc` -> alias teardown and its leak check. `middle_policy_stage.inc` now sequences those two boundaries only. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Completed in `cleanup-1786784343365`: split the historical opening continuation into `composition/opening_state_completion_stage.inc` and split the late legacy chain into `composition/late_execution_stage.inc` plus `composition/late_registry_reporting_stage.inc`. The opening chain still expands `part_003.inc` -> `part_004.inc` -> `part_005.inc` -> Garbodor policy with identical alias lifetimes. The late chain still expands `part_014c.inc` -> `part_015.inc` -> `part_016.inc`, including the cross-fragment `play_field_blower`, `run_turn`, and scenario aliases. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Completed in `cleanup-1786782616617`: the thin `composition/middle_policy_stage.inc` sequencer now has a single-composition guard while leaving the banked-Tapu and lock-removal macro contracts inside their named substages. This keeps the sequencer ownership explicit without changing textual order or strategy ownership. C++ textual-include semantics: https://eel.is/c++draft/cpp.include Canonical composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

Completed in `cleanup-1786786017579`: `composition/opening_legacy_stage.inc` now asserts the two intentional cross-stage exports, `begin_turn` and `ability_available_for_pokemon`, after its opening continuation is composed. `composition/late_legacy_stage.inc` now validates that the execution substage releases all temporary aliases, including `play_field_blower`, `run_turn`, `hand_count`, `play_items_until_stable`, and `ability_available_for_pokemon`, before namespace-scope registry/reporting composition begins. These are compile-time composition contracts only; include order, declaration order, gameplay policy, DCI/UDP/AMR, K0/K1, lock timing, and route decisions are unchanged. C++ conditional preprocessing: https://eel.is/c++draft/cpp.cond Opening owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_stage.inc Late owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_legacy_stage.inc Historical execution owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

Next composition step: inventory `composition/opening_engine_overrides.inc` for one contiguous alias-owned seam whose `#define` / `#include` / `#undef` lifetime can move into a named substage without crossing strategy decisions. Keep gameplay route decisions in their current policy owners and keep each alias setup attached to the include/teardown sequence it protects.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

`PayloadZonePolicy::first()` preserves physical zone order for callers whose historical behavior depends on the first matching card in that zone. `PayloadZonePolicy::contains()` and `PayloadZonePolicy::count()` own zone membership and count semantics. `PayloadPreferencePolicy::first_preferred()` preserves the explicit strategic preference order used for payload selection, while `PayloadPreferencePolicy::first_preferred_with_positive_count()` adapts count-backed zones without duplicating preference traversal.

Completed in `cleanup-1786767958859`: renamed the preference-order helper to `first_preferred()` and clarified zone predicates as `contains_in_zone()` and `count_in_zone()`. These are behavior-preserving ownership changes. DCI/JIT admission and payload route policy remain at their existing Engine callers. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Completed in `cleanup-1786781829278`: split physical-zone queries from strategic payload preference under `PayloadZonePolicy` and `PayloadPreferencePolicy`, then centralized count-backed hand and K1-deck preference selection behind `first_preferred_with_positive_count()`. These are behavior-preserving ownership changes. DCI/JIT admission, K0/K1 timing, payload discard timing, and route policy remain at their existing Engine callers. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Completed in `cleanup-1786784426003`: `PayloadZonePolicy` now routes both presence checks and first-card retrieval through one physical-order `std::find_if` seam, while `PayloadPreferencePolicy::first_preferred_in_zone()` owns direct strategic-priority membership lookup for concrete zones such as the hand. The change removes repeated payload traversal from Engine-facing wrappers without changing the five-card preference order, physical-zone ordering, DCI/JIT admission, K0/K1 timing, discard timing, or route decisions. C++ find semantics: https://eel.is/c++draft/alg.find Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 Decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

Next payload step: inventory remaining ad hoc Dragon-payload scans and migrate only callers whose ordering semantics exactly match `PayloadZonePolicy::first()`, `PayloadPreferencePolicy::first_preferred_in_zone()`, or the explicit count-backed preference order. Preserve physical-order callers when order is observable. Keep DCI/JIT predicates and discard timing at their strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations. `src/trace_engine_v2/core/forretress/runtime.inc` owns runtime composition and route-facing definitions. `src/trace_engine_v2/core/forretress/exploding_energy_runtime.inc` owns the contiguous printed Exploding Energy resolver, Forretress-stack discard helper, board-index target adapter, and immediate post-KO promotion resolver. The declaration/runtime split remains necessary while Engine is a textual class body and runtime definitions are emitted after Engine closes.

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery through `board_index_for_card()`, deterministic ranked board queries, and the prior-turn evolution timing predicate. `board_index_best_matching()` preserves first-equal ordering by replacing the selected index only when a later candidate ranks strictly higher, while `board_index_is_benched()` and `best_benched_pokemon_index()` centralize Bench-only ranking. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc C++ `max_element` ordering: https://eel.is/c++draft/alg.sorting#alg.max.min

Completed through `cleanup-1786767670470`: Forretress ex source discovery, Active-role checks, board index conversion, destination lookup, and shared traversal use the canonical board-state vocabulary. Attachment selection, self-Knock-Out, promotion, retreat payment, DCI/UDP/AMR, K0/K1, readiness, and route policy remain at their prior owners.

Completed in `cleanup-1786769773419`: named the Bench board-index role and added deterministic ranked board selection over the existing Active-first visitor. The new seam retains first-equal ordering and does not move the Forretress strategy comparator or change card legality, promotion, retreat payment, attachment distribution, self-Knock-Out, DCI/UDP/AMR, K0/K1, readiness, or route policy.

Completed in `cleanup-1786778220000`: both Forretress Bench selectors now route through `best_benched_pokemon_index()`. The self-Knock-Out promotion keeps its existing semantic Apex-readiness ranking, and the post-Ability retreat target keeps its existing Regidrago-stage, attached-Energy, and entry-turn ranking. Both comparators remain in the Forretress strategy owner. The shared selector replaces a candidate only on a strict rank win, preserving the former `std::max_element` first-equal tie result. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc Forretress runtime owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc C++ `max_element` ordering: https://eel.is/c++draft/alg.sorting#alg.max.min

Completed in `cleanup-1786784386299`: extracted the contiguous Exploding Energy resolution cluster into `core/forretress/exploding_energy_runtime.inc` and composed it from `runtime.inc` at the identical definition boundary. This keeps the printed search/attachment/shuffle/self-Knock-Out resolution, Forretress-stack discard, board-index target adapter, and immediate post-KO promotion mechanics together while leaving setup admission, retreat planning, DCI/UDP/AMR, K0/K1, readiness, and route priority in the existing Forretress strategy owner. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Core rules: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

Completed in `cleanup-1786782616617`: the Forretress board-role contract now delegates Regidrago V-line identity to the canonical `is_regidrago_v_line()` classifier. The Forretress-specific board-role predicate remains available to runtime callers while intrinsic Regidrago V/VSTAR identity stays owned by `core/card_classification.inc`. Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Canonical classifier: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc

Next mechanical Forretress step: inventory the remaining orchestration in `runtime.inc` for another exact semantic boundary. Keep `use_exploding_energy_for_setup()` with strategy-owned attachment distribution and retreat planning until an extraction can preserve its route ordering exactly, and keep DCI/UDP/AMR, K0/K1, readiness, and route policy outside generic rules helpers. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117 Core evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

## Steven route cleanup

Named Steven route owners live under `src/trace_engine_v2/core/routes/`. The issue-3653 free-slot connector and issue-3202 VSTAR/Vessel route are composed directly from their canonical owners. The issue-3203 Active-VSTAR route is composed at its established nested parent boundary. Historical forwarders for those routes are retired.

The root composition inventory is now easier to reason about because opening and late historical chains have named stage owners. Continue the Steven inventory only at a verified class-member boundary. A Steven-named root fragment may be retired when it contains composition-only forwarding and its canonical `core/routes/` owner can replace it at that exact textual boundary. Preserve route admission, DCI/UDP/AMR, Supporter contention, connector domination, and source URLs. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels together with opening-hand, mulligan, Prize-deal, and setup-trace mechanics. `src/trace_engine_v2/part_005.inc` now composes that canonical owner directly at the established Engine member boundary.

Completed in `cleanup-1786770665212`: removed the redundant `core/simulation_labels.inc` composition edge from `part_005.inc` and retired the empty compatibility shell after confirming the canonical setup lifecycle was already included at the same textual boundary. This is a structure-only cleanup; setup choice policy, K0/K1 state, mulligan counting, Active/Bench selection, Prize placement, declaration order, DCI/UDP/AMR behavior, Supporter contention, and connector domination are unchanged. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/setup_lifecycle.inc C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Completed in `cleanup-1786771204658`: grouped DCI/lock display vocabulary under `SetupLabelPolicy` and replaced the local recipe-classification lambda with the named `setup_recipe_contains()` seam. These are behavior-preserving ownership changes inside the canonical setup lifecycle owner. DCI/JIT vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment Scenario lock vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment

Completed in `cleanup-1786773206443`: centralized K0/K1 copy-count arithmetic under `KnowledgeCopyPolicy` and grouped setup constants with scenario-label vocabulary under `SetupLifecycleConfig`. The changes keep public-zone knowledge, fixed-list unresolved-copy math, mulligan size, Prize count, DCI/JIT labels, lock labels, and trace output semantics at their established owners. Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Completed in `cleanup-1786773358132`: added `setup_recipe_has_exactly()` so exact recipe classification uses the setup query seam consistently, and added `setup_scenario_summary()` so the trace emitter no longer assembles DCI/lock display vocabulary inline. These are behavior-preserving centralization changes layered on `SetupLifecycleConfig`; opening Active/Bench strategy, K0/K1 state, mulligan and Prize mechanics, DCI/UDP/AMR, Supporter contention, connector domination, and gameplay rules remain unchanged. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/setup_lifecycle.inc

Next setup step: move only state-transition helpers from opening Active/Bench setup into `core/setup_lifecycle.inc` once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Keep the strategic route predicates in Engine so DCI/UDP/AMR, Supporter contention, connector domination, and lock-aware choice remain policy-owned. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario/timing: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
