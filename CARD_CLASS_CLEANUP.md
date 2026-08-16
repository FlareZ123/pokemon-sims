# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup-wave notes belong in Git history. Keep this file limited to current ownership, remaining work, and validation requirements.

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

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokemon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated names and intrinsic-classification compatibility. Registered metadata lookup remains first and canonical.

Next catalog step: migrate remaining compatibility entries one card at a time. Delete a compatibility row only after an explicit `CardDefinition`, registration, exact-print source, and focused metadata test exist. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

Regidrago VSTAR owns exact Silver Tempest 136/195 metadata beside Regidrago V and is explicitly registered. Exact prints: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136

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

If migration exposes incorrect gameplay, use the bug-confirmation workflow instead of combining the gameplay fix with cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. Composition-only macro lifetimes belong there or at the established named composition boundary. Semantic route policy belongs under `src/trace_engine_v2/core/routes/`.

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual include semantics: https://eel.is/c++draft/cpp.include

The root `part_000.inc` and `part_001.inc` compatibility paths remain because unified-test/source-contract tooling reads them directly. `part_000.inc` exposes the catalog include needed by unified-test header discovery, while `part_001.inc` preserves the non-executable payload predicate mirror expected by raw-source contracts. Unified-test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

The issue-1393 held-Crispin compatibility seam is retired. `src/trace_engine_v2/core/routes/crispin_supported_route_policy.inc` is the semantic owner and the live composition boundary now includes it directly. Crispin: https://api.pokemontcg.io/v2/cards/sv7-133 Double Dragon Energy: https://api.pokemontcg.io/v2/cards/xy6-97 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

The issue-1516/2164 Quick Ball, Tapu Lele-GX, Crispin route family has canonical semantic ownership at `src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc`. `src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc` remains a minimal compatibility composition seam at the established Quick Ball wrapper boundary. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60 Crispin: https://api.pokemontcg.io/v2/cards/sv7-133

Next composition step: retire the Quick Ball/Tapu/Crispin compatibility seam once its live consumer can include the canonical owner directly at the identical wrapper boundary. Inspect another root `part_*` seam only when its complete macro lifetime or function body can move intact. Preserve declaration order, route semantics, and direct source URLs.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Reuse `PayloadZonePolicy::contains()`, `contains_card()`, `contains_matching()`, `count()`, and `count_matching()` only when semantics match exactly. Preserve physical-order selection where order is observable and explicit strategic order where preference is required.

Keep DCI/JIT predicates and discard timing at strategy owners. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine declarations and card-facing board-role classifiers for Pineco, Forretress ex, the Pineco -> Forretress ex line, and the Regidrago V line.

`src/trace_engine_v2/core/forretress/runtime.inc` now owns the complete Forretress runtime, including the printed Exploding Energy resolver, Forretress-stack discard transition, board-index target adapter, immediate post-KO promotion, setup orchestration, and search connectors. The forwarding split `src/trace_engine_v2/core/forretress/exploding_energy_runtime.inc` is retired. State mutation, attachment distribution, Knock Out handling, promotion ranking, and direct source URLs remain unchanged. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Core Ability/search/attachment/Knock Out procedure: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery, deterministic ranked board queries, and prior-turn evolution timing. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc

Next Forretress step: inventory adjacent root route fragments for another complete semantic boundary. Replace board-object identity or prior-turn evolution checks only when their semantics exactly match existing board-policy classifiers. Preserve state-count queries, Forest of Vitality entry-turn timing, route ordering, attachment distribution, retreat planning, and strategic ranking. Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117

## Steven route cleanup

Named Steven route policies live under `src/trace_engine_v2/core/routes/`. Reuse `resolve_gladion_prize_exchange()` only after a caller has completed legal Prize reveal and target selection with identical semantics. Reuse `item_locked_on_turn()` for shared persistent Item-lock timing.

Keep route admission, DCI/UDP/AMR, Supporter contention, connector domination, hidden-information sequencing, target ranking, and direct source URLs at their current owners. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Gladion: https://api.pokemontcg.io/v2/cards/sm4-95 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing labels, opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup trace output. `SetupRecipePolicy` owns setup recipe-presence and exact-count predicates.

Next setup step: move only state-transition helpers from opening Active/Bench setup once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/deck_knowledge.inc` keeps copy arithmetic behind `KnowledgeCopyPolicy`. `combined()` owns two-source aggregation. `combined_unattached_public_zones()` owns the shared hand-plus-discard public-zone base. `combined_public_zones()` composes that base with attached public copies. K1 hand/deck counts continue to reuse `combined()`. K0/K1 visibility rules remain at Engine callers: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: move repeated copy arithmetic into `KnowledgeCopyPolicy` only after the Engine caller has resolved visibility. Do not recreate a second classification fragment, forwarding-only catalog lookup members, or duplicate registered Retreat Cost/name metadata in compatibility switches. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`.
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a loop or route-local helper, check these owners and reuse a named seam only when ordering and semantics match exactly.

## Projection cleanup

Prefer named pure-projection members over route-local anonymous lambdas when the projection is reused or carries a distinct policy contract. Merge a root fragment into a canonical semantic owner only when its complete body or macro lifetime can move at the identical textual boundary. Keep physical resolution, trace emission, K0/K1 transitions, strategic route choice, and source URLs at their current owners.

## Validation gate

A cleanup PR is mergeable only when:

- strict Release compilation succeeds;
- focused tests and the full regression suite show no new failure;
- sanitizer and structural checks show no new failure;
- representative `--simulate-this` traces preserve legal action ordering and readiness;
- the paired T2/T3 matrix has no unexplained drift; and
- the PR contains no gameplay behavior change.

Known baseline failures must be tied to an existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of being folded into cleanup.