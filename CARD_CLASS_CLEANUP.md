# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup-wave notes belong in Git history. Keep this file focused on current ownership, remaining work, and validation requirements.

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
src/trace_engine_v2/core/adapters/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

The old `src/trace_engine_v2/core/card_context_adapter.hpp` forwarding include is retired. Repository consumers must include the canonical adapter owner directly.

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokemon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` is the sole trace-engine construction bridge for reusable card effects.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` remains the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains the first metadata path.

Next catalog step: migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time. Delete a compatibility row only after that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

Regidrago VSTAR owns exact Silver Tempest 136/195 metadata beside Regidrago V in `src/cards/pokemon/regidrago_v.hpp`, is explicitly registered, and has focused V/VSTAR metadata/parity coverage. Exact prints: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Pokemon V ruling: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/

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

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It owns simulator runtime inclusion, the opening `part_003.inc` -> `part_004.inc` -> `part_005.inc` continuation, banked-Tapu and lock-removal alias lifetimes, and the late `part_014c.inc` -> `part_015.inc` -> `part_016.inc` continuation. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc Runtime state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

`src/trace_engine_v2/composition/steven_blender_overrides.inc` owns the contiguous Steven/Brilliant Blender macro-composition block formerly embedded in `opening_engine_overrides.inc`. Route admission remains with the existing `core/routes/` owners. Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/steven_blender_overrides.inc

The root `part_000.inc` and `part_001.inc` compatibility paths remain because unified-test/source-contract tooling reads them directly. `part_000.inc` is the single legacy catalog include shim, and `part_001.inc` delegates catalog inclusion through it while preserving the non-executable payload predicate mirror expected by raw-source contracts. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Unified-test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

Retired route forwarding seams include the issue-1393 Crispin helper and issue-1516/2164 Quick Ball/Tapu/Crispin wrapper. Their canonical owners are `core/routes/crispin_supported_route_policy.inc` and `core/routes/quick_ball_tapu_crispin_policy.inc`. Crispin: https://api.pokemontcg.io/v2/cards/sv7-133 Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60

The issue-1368 Earthen Vessel / Celestial Roar route has a canonical semantic owner at `src/trace_engine_v2/core/routes/earthen_vessel_celestial_roar_policy.inc`. Its historical root compatibility include remains until the post-`part_014a` composition boundary can point directly at the canonical owner without changing the pre-DDE macro exports consumed by the issue-2437 layer. Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163 Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Next composition step: prove raw-source/tooling consumers of the issue-1368 historical root path absent, rewire the identical post-`part_014a` macro boundary directly to the canonical route owner, then retire that forwarding include. Inspect another root `part_*` seam only when its complete macro lifetime or function body can move intact. Keep tooling-only compatibility paths minimal and retain direct source URLs beside rule-sensitive logic.

## Shared strategy and runtime owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Board traversal, `BoardIndex`, attachment destination, and evolution-age queries: `src/trace_engine_v2/core/board_state_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.
- Per-turn action and transient lock resets: `src/trace_engine_v2/core/turn_lifecycle.inc`.
- K0/K1 copy arithmetic after visibility is resolved: `src/trace_engine_v2/core/deck_knowledge.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

## Route cleanup

Named Steven route policies live under `src/trace_engine_v2/core/routes/`. `gladion_steven_route_policy.inc` owns the shared `resolve_gladion_prize_exchange()` state transition after legal Prize reveal and target selection. `active_vstar_steven_route_policy.inc` delegates projected Item legality to the canonical `item_locked_on_turn()` seam and names the exact projected Treasure turn. Route overlays retain admission, target choice, hidden-information sequencing, DCI/JIT policy, and trace text. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Gladion: https://api.pokemontcg.io/v2/cards/sm4-95 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Shared lock timing: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

Next route step: migrate duplicated Gladion Prize-exchange mutations to `resolve_gladion_prize_exchange()` only when reveal and target-selection semantics match exactly. Continue retiring composition-only `part_*steven*` forwarders whose canonical `core/routes/` owner can replace them at the identical textual boundary.

## Payload cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` owns physical-zone payload traversal and explicit preference traversal. Replace remaining ad hoc Dragon-payload membership/cardinality scans only where semantics exactly match an existing policy operation. Preserve physical order when observable, explicit strategic order where required, and DCI/JIT timing at strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations and card-facing board-role classifiers. `src/trace_engine_v2/core/forretress/runtime.inc` owns the complete Forretress runtime. `src/trace_engine_v2/core/forretress/scenario_extension.inc` owns reusable scenario append/lookup mechanics. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Official Ability/search/attachment/Knock Out procedure: https://www.pokemon.com/us/pokemon-tcg/rules

Next Forretress step: inventory remaining orchestration in `runtime.inc` and adjacent root route fragments for another complete semantic boundary. Preserve state-count queries, entry-turn evolution timing, route ordering, attachment distribution, retreat planning, and strategic ranking at their existing owners.

## Projection cleanup

Prefer named pure-projection members over route-local anonymous lambdas when a projection is reused or carries a distinct policy contract. Merge a remaining root fragment into a canonical semantic owner only when its complete function body or macro lifetime can move at the identical textual boundary. Keep physical resolution, trace emission, K0/K1 transitions, strategic route choice, and source URLs at their current owners.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.
