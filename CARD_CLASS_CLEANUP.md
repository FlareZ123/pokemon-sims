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

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokemon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains the first metadata path.

Next catalog step: migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time. Delete a compatibility row only after that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

Regidrago VSTAR owns exact Silver Tempest 136/195 metadata beside Regidrago V in `src/cards/pokemon/regidrago_v.hpp`, is explicitly registered, and has focused V/VSTAR metadata/parity coverage. The live Pokemon, Pokemon V, Rule Box, Dragon/Mysterious Treasure target, and Retreat Cost classifiers consume registered metadata for the Regidrago line. Exact prints: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Pokemon V ruling: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It owns the simulator runtime inclusion, opening `part_003.inc` -> `part_004.inc` -> `part_005.inc` continuation, banked-Tapu and lock-removal alias lifetimes, and late `part_014c.inc` -> `part_015.inc` -> `part_016.inc` continuation. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc Runtime state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The root `part_000.inc` and `part_001.inc` compatibility paths remain because unified-test/source-contract tooling reads them directly. `part_000.inc` exposes the catalog include needed by unified-test header discovery, while `part_001.inc` preserves the non-executable payload predicate mirror expected by raw-source contracts. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Unified-test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

The issue-1393 held-Crispin completion helper has a canonical semantic owner at `src/trace_engine_v2/core/routes/crispin_supported_route_policy.inc`. The historical root `part_issue_1393_crispin_route_helper.inc` seam is retired after the live `part_issue_1356_fss_energy_override.inc` composition boundary was rewired directly to the canonical owner and no source-contract or generator consumer remained. Route code, DDE-aware projection, K1/JIT policy, and direct sources remain together under `core/routes/`. Crispin: https://api.pokemontcg.io/v2/cards/sv7-133 Double Dragon Energy: https://api.pokemontcg.io/v2/cards/xy6-97 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/crispin_supported_route_policy.inc

The issue-1516/2164 Quick Ball, Tapu Lele-GX, Crispin route family now has a canonical semantic owner at `src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc`. Its internal route helpers are named for the behavior they implement rather than historical issue numbers; issue IDs remain only where trace/provenance text or source links intentionally preserve debugging history. The historical `part_issue_1516_quick_ball_tapu_crispin_override.inc` path remains a minimal compatibility composition seam that includes the canonical owner at the existing Quick Ball wrapper boundary. Route admission, copied-Engine projection, K1 checks, lock checks, trace text, and direct source URLs remain unchanged. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60 Crispin: https://api.pokemontcg.io/v2/cards/sv7-133 Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc

Next composition step: retire the Quick Ball/Tapu/Crispin compatibility seam once its live composition consumer can include the canonical owner directly at the identical wrapper boundary. Inspect another root `part_*` seam only when its complete macro lifetime or function body can move intact. Keep tooling-only compatibility paths minimal, preserve declaration order and route semantics, and retain direct source URLs beside rule-sensitive logic.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

- `PayloadZonePolicy::first_iterator_matching()` owns shared physical-zone first-match traversal.
- `PayloadZonePolicy::contains_matching()` owns generic predicate-based zone membership.
- `PayloadZonePolicy::count_matching()` owns generic predicate-based zone cardinality.
- `PayloadZonePolicy::first()` preserves physical zone order for callers whose historical behavior depends on first match.
- `PayloadZonePolicy::contains()` and `PayloadZonePolicy::count()` own generic payload membership/count semantics.
- `PayloadZonePolicy::contains_card()` owns concrete-card physical-zone membership.
- `PayloadPreferencePolicy::first_preferred()` preserves explicit strategic priority.
- `PayloadPreferencePolicy::first_preferred_in_zone()` composes preference order with physical-zone membership.
- `PayloadPreferencePolicy::first_preferred_with_positive_count()` adapts count-backed zones without duplicating preference traversal.

The #2408 Burnet-versus-Serena held-Dragon check delegates to `payload_zone_contains(state_.hand)`. The #2271 surplus-Regidrago route delegates its exclusion-aware hand scan to `PayloadZonePolicy::contains_matching()` while preserving the `Card::RegidragoV` exclusion. Burnet route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_2408_burnet_resource_override.inc Surplus route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_2271_surplus_regidrago_v_route_override.inc

Next payload step: replace remaining ad hoc Dragon-payload membership and cardinality scans only where semantics exactly match an existing `PayloadZonePolicy` operation. Preserve physical-order selection when order is observable and preserve explicit strategic order where preference is required. Keep DCI/JIT predicates and discard timing at strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations and card-facing board-role classifiers for Pineco, Forretress ex, the combined Pineco -> Forretress ex line, and the Regidrago V line. `src/trace_engine_v2/core/forretress/runtime.inc` owns runtime composition and route-facing definitions. `src/trace_engine_v2/core/forretress/exploding_energy_runtime.inc` owns the printed Exploding Energy resolver, Forretress-stack discard helper, board-index target adapter, and immediate post-KO promotion resolver. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery, deterministic ranked board queries, and prior-turn evolution timing. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc

Next mechanical Forretress step: inventory remaining orchestration in `runtime.inc` and adjacent root route fragments for another complete semantic boundary. Reuse board-policy classifiers only where semantics match exactly. Preserve state-count queries, entry-turn evolution timing, route ordering, attachment distribution, retreat planning, and strategic ranking at their existing owners. Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117 Core evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Steven route cleanup

Named Steven route policies live under `src/trace_engine_v2/core/routes/`. `core/routes/gladion_steven_route_policy.inc` owns the shared `resolve_gladion_prize_exchange()` state transition after legal Prize reveal and target selection. Route overlays retain admission, target choice, hidden-information sequencing, DCI/JIT policy, and trace text. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Gladion: https://api.pokemontcg.io/v2/cards/sm4-95 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Projected Item-lock timing delegates to the canonical Engine `item_locked_on_turn()` seam instead of re-encoding lock-family identities inside route files. Shared timing owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

Next Steven cleanup step: migrate duplicated Gladion Prize-exchange mutations to `resolve_gladion_prize_exchange()` only when reveal and target-selection semantics match exactly. Continue retiring composition-only `part_*steven*` forwarders whose canonical `core/routes/` owner can replace them at the identical textual boundary.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels, opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that owner at the established Engine member boundary. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

`SetupRecipePolicy` owns setup recipe-presence and exact-count predicates. Opening-deck and mulligan transitions remain directly owned by their lifecycle procedures rather than one-use forwarding helpers.

Next setup step: route future setup recipe classification through `SetupRecipePolicy`. Move state-transition helpers from opening Active/Bench setup only once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Keep strategic route predicates in Engine.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` owns the shrinking legacy name bridge and intrinsic classification compatibility seam. Registered `CardDefinition` lookup remains canonical for migrated names and intrinsic metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

`src/trace_engine_v2/core/deck_knowledge.inc` keeps copy arithmetic behind `KnowledgeCopyPolicy`. K0/K1 visibility rules remain at Engine callers: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: migrate legacy name and intrinsic metadata rows only after explicit `CardDefinition` registration and coverage. Move repeated copy-count arithmetic into `KnowledgeCopyPolicy` only after visibility has been resolved by the Engine caller. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

## Turn lifecycle cleanup

`src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn resets. `TurnActionStatePolicy::reset()` clears generic action flags and same-turn discard tracking. `TransientTurnLockPolicy::reset()` owns scenario-dependent one-turn Garbodor unlock reset. Established order remains: set turn, clear action state, restore transient lock pressure, then perform the mandatory start-of-turn draw. Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104 Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next turn-lifecycle step: route exact duplicate action-flag/reset bundles through `TurnActionStatePolicy::reset()` and exact scenario-scoped transient lock resets through `TransientTurnLockPolicy::reset()`. Preserve ordering relative to the required turn draw, and keep persistent matchup state outside these per-turn owners.

## Projection cleanup

`src/trace_engine_v2/composition/post_014a_overrides.inc` gives the Tate public-projection recursion guard a named Engine member type. The projection isolates Legacy Star and restores the same thread-local depth on scope exit: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

`src/trace_engine_v2/part_roseanne_multimode_override.inc` evaluates the Evolution Incense -> Earthen Vessel admission path on a copied `Engine`, matching the neighboring Pokemon Communication projection and avoiding temporary mutation/restoration of live hand state. Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148 Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163 Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163

Next projection step: prefer named pure-projection members over route-local anonymous lambdas when a projection is reused or carries a distinct policy contract. Merge a remaining root fragment into a canonical semantic owner only when its complete function body or macro lifetime can move at the identical textual boundary. Keep physical resolution, trace emission, K0/K1 transitions, strategic route choice, and source URLs at their current owners.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
