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

Quick Ball is the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility name bridge for cards that have not yet migrated into `CardDefinition`. Registry lookup remains the first and canonical name path.

Next catalog step: migrate remaining `LegacyCardCatalog` entries one card at a time through the normal ownership workflow. Delete a compatibility row only after that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It sequences `opening_legacy_stage.inc`, `banked_tapu_policy_stage.inc`, `lock_removal_policy_stage.inc`, then `late_engine_stage.inc` at the established textual boundaries.

`composition/late_engine_stage.inc` owns the complete historical `part_014c.inc` -> `part_015.inc` -> `part_016.inc` chain because `play_field_blower` and `run_turn` intentionally span those fragments. Keeping setup, continuation, and teardown in one stage makes the real macro lifetime the ownership boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_engine_stage.inc

`composition/opening_legacy_stage.inc` directly owns the complete historical `part_003.inc` -> `part_004.inc` -> `part_005.inc` opening chain, the Garbodor Ability alias, and the temporary opening-deck visibility alias teardown. The former forwarding-only `opening_state_completion_stage.inc` has been retired so the real cross-fragment macro lifetime is visible in one owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_legacy_stage.inc

The banked-Tapu and lock-removal stages remain separate because each owns a complete local alias setup/include/teardown boundary.

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission/projection/decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Next composition step: inventory `opening_legacy_stage.inc` for another complete semantic boundary only after exact source-contract coverage exists for the remaining cross-fragment aliases. Do not recreate `opening_state_completion_stage.inc` or another forwarding-only sequencer.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

- `PayloadZonePolicy::first_iterator_matching()` owns the shared physical-zone first-match traversal primitive. Payload and exact-card membership build on this seam so they cannot drift into separate scan implementations.
- `PayloadZonePolicy::contains_matching()` owns generic predicate-based zone membership and keeps boolean membership checks on the same traversal primitive.
- `PayloadZonePolicy::count_matching()` owns generic predicate-based zone cardinality so payload counts do not grow independent `std::count_if` scans.
- `PayloadZonePolicy::first()` preserves physical zone order for callers whose historical behavior depends on the first matching payload.
- `PayloadZonePolicy::contains()` and `PayloadZonePolicy::count()` own generic payload membership/count semantics.
- `PayloadZonePolicy::contains_card()` owns concrete-card membership in a physical zone so preference code does not duplicate `std::find` scans.
- `PayloadPreferencePolicy::first_preferred()` preserves the explicit five-card strategic priority.
- `PayloadPreferencePolicy::first_preferred_in_zone()` composes preference order with physical-zone membership.
- `PayloadPreferencePolicy::first_preferred_with_positive_count()` adapts count-backed zones without duplicating preference traversal.

Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next payload step: replace remaining ad hoc Dragon-payload membership and cardinality scans only where semantics exactly match `PayloadZonePolicy::contains()`, `contains_card()`, `contains_matching()`, `count()`, or `count_matching()`. Preserve physical-order selection when order is observable and preserve the explicit strategic order where preference is required. Keep DCI/JIT predicates and discard timing at strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations and the card-facing board-role classifiers for Pineco, Forretress ex, the combined Pineco -> Forretress ex line, and the Regidrago V line. `src/trace_engine_v2/core/forretress/runtime.inc` owns runtime composition and route-facing definitions. `src/trace_engine_v2/core/forretress/exploding_energy_runtime.inc` owns the contiguous printed Exploding Energy resolver, Forretress-stack discard helper, board-index target adapter, and immediate post-KO promotion resolver. Promotion code reuses `is_regidrago_board_pokemon()` instead of maintaining a second Regidrago-family membership check. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery, deterministic ranked board queries, and the prior-turn evolution timing predicate. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc

Next mechanical Forretress step: replace remaining direct Pineco / Forretress ex board-card identity checks only where they exactly match the contract classifiers, then inventory the remaining orchestration in `runtime.inc` for another complete semantic boundary. Preserve state-count queries, entry-turn evolution timing, route ordering, attachment distribution, retreat planning, and strategic ranking at their existing owners. Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117 Core evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

## Steven route cleanup

Named Steven route owners live under `src/trace_engine_v2/core/routes/`. Retire a Steven-named root fragment only when it contains composition-only forwarding and its canonical route owner can replace it at the identical textual boundary. Preserve route admission, DCI/UDP/AMR, Supporter contention, connector domination, and source URLs. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels together with opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that canonical owner at the established Engine member boundary.

`SetupRecipePolicy` owns setup recipe-presence and exact-count predicates used by deck/scenario classification. `prepare_opening_deck()` owns knowledge reset, recipe population, and the opening shuffle. `draw_opening_hand_once()` owns the repeated seven-card transfer used by the mulligan loop. Scenario summaries call `SetupLifecycleConfig` directly instead of retaining forwarding-only label wrappers. `SetupLifecycleConfig::kUnknownLabel` now centralizes the fallback label shared by DCI and lock rendering, keeping setup display vocabulary in one owner. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

Next setup step: route future setup recipe classification through `SetupRecipePolicy` and move only state-transition helpers from opening Active/Bench setup into `core/setup_lifecycle.inc` once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Keep strategic route predicates in Engine.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` keeps legacy name metadata behind `LegacyCardCatalog`. `LegacyCardCatalog::find()` is the single fallback-table traversal, and `name()` now falls directly through to `LegacyCardCatalog::name()` after the registered `CardDefinition` lookup. Registered metadata remains canonical: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

The global `name(Card)` compatibility seam now falls directly from registered `CardDefinition` lookup to `LegacyCardCatalog::name()` without a forwarding-only legacy helper. Keep future metadata cleanup on those two owners rather than adding another name-routing layer.

`src/trace_engine_v2/core/deck_knowledge.inc` keeps copy arithmetic behind `KnowledgeCopyPolicy`. `KnowledgeCopyPolicy::combined()` owns repeated two-source count aggregation for public hand/discard/attached zones and K1 hand/deck counts. K0/K1 visibility rules remain unchanged at their Engine callers: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: move only duplicate metadata lookup or copy-count arithmetic into these helpers. Do not add forwarding-only wrappers around the legacy catalog fallback. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario/timing: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. `GarbodorScenarioPolicy::activation_turn_reached()` owns the shared turn-threshold check, and `GarbodorScenarioPolicy::active()` composes that timing with scenario identity. Engine wrappers remain compatibility/query seams for callers that need the individual facts. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

Next shared-policy step: replace duplicated Garbodor activation-turn comparisons only when they are semantically identical to `GarbodorScenarioPolicy::activation_turn_reached()`, and replace scenario-prefix plus activation conjunctions only when they are semantically identical to `GarbodorScenarioPolicy::active()`. Preserve separate `matches()` and `lock_activation_turn()` queries where callers need one fact without the other.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
