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
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains the first and canonical metadata path.
- `src/trace_engine_v2/composition/engine_body.inc` and `composition/post_014a_overrides.inc` own textual composition and macro lifetimes. Semantic route policy should move under `core/` or `core/routes/` without moving its established member boundary until source-contract coverage proves that safe.

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

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Current canonical owners include:

- Runtime and ordered Engine composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
- Late override composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
- Intrinsic compatibility catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
- Board-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
- Setup lifecycle: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/setup_lifecycle.inc
- Turn lifecycle: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/turn_lifecycle.inc
- Dragon payload queries: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
- Deck-knowledge arithmetic: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc
- Garbodor/Rule Box Ability-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/garbodor_lock_policy.inc
- Recovery Supporter policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/recovery_supporter_policy.inc
- Turn-action runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/turn_action_policy_runtime.inc

Before adding a route-local loop, classifier, copy-count helper, or lock predicate, check these owners and reuse a named seam when ordering and semantics match exactly.

## 2026-08-15 semantic-owner checkpoint

This cleanup pass advances the root-`.inc` reduction while preserving gameplay behavior.

- `src/trace_engine_v2/core/steven_blender_semantic_policy.inc` is now the central semantic owner for the former #3221 K0 and #3222 K1 Steven's Resolve + Brilliant Blender route predicates. The historical root files remain narrow forwarding seams so the established class-member positions and K0/K1 section boundaries stay unchanged. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- `src/trace_engine_v2/core/crispin_route_policy.inc` is now the semantic owner for the held-Crispin completion predicate. The historical root helper remains a forwarding seam at the identical member boundary. Crispin: https://api.pokemontcg.io/v2/cards/sv7-133 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Both moves keep route admission, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1, readiness, and payload timing inside simulator strategy. They do not migrate strategic decisions into card code.
- Direct card/rule/source URLs remain beside the moved implementations, so later card-class extraction can distinguish printed legality from strategic admission.

Next composition step: replace these forwarding seams at their composition call sites only after the exact include/macro lifetime can be preserved directly and source-contract tests cover the boundary. Continue retiring one-purpose root `part_*` fragments by moving complete semantic bodies into existing `core/` owners. Avoid introducing new forwarding-only sequencers.

## Catalog and card-class next steps

1. Migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time. Delete a compatibility row only after the card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test.
2. Keep Regidrago V/VSTAR evolution relations, Legacy Star, Apex Dragon, DCI/JIT, and route choice at behavioral owners while metadata lookup continues moving through the registry. Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
3. Move repeated physical-zone Dragon payload membership/cardinality checks through `PayloadZonePolicy` only when physical ordering and preference semantics match exactly.
4. Route repeated public/known copy arithmetic through `KnowledgeCopyPolicy` only after the Engine caller has resolved hidden-zone visibility. K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
5. Keep projected persistent Item-lock checks on the canonical `item_locked_on_turn()` seam when their semantics match the shared schedule. Lock rules and scenario semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
