# Card Class Cleanup

This file is the current-state architecture and migration plan. Historical cleanup-wave notes remain available in Git history. Keep entries focused on live ownership, remaining work, and validation requirements.

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

Quick Ball is the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 -> K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture contracts

### `card_id.hpp`

`src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Do not add a second ID system or renumber existing enumerators during cleanup. Exact external print identity belongs in `CardDefinition::canonical_id`.

### `card_definition.hpp`

`CardDefinition` owns intrinsic exact-print facts: display name, canonical print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and a direct source URL.

Payload role, DCI/UDP, AMR, strict-JIT value, route priority, matchup policy, Supporter contention, connector domination, K0/K1 state, and setup-axis value belong in simulator strategy.

### `card_registry.hpp`

Registration is explicit and deterministic. `kRegisteredCardDefinitions` is the canonical inventory, and `find_definition()` is the canonical lookup. Helper predicates derive from that definition instead of maintaining parallel registration switches.

Current registry source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

### `card_context.hpp`

`CardContext` is the reusable printed-rules seam. Add only general game operations needed by card effects. Card-specific route-policy queries stay outside this interface.

Knowledge transitions, zone mutations, shuffle behavior, and trace ordering must stay compatible with the simulator unless a separately confirmed bug authorizes a behavior change.

## Card module contract

A migrated card gets one primary module under `src/cards/pokemon/`, `src/cards/trainers/`, or `src/cards/energy/`.

Card modules own exact-print metadata, printed action/choice shape, intrinsic cost legality, printed target categories/cardinality, printed resolution through `CardContext`, and direct source identity.

Engine/strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, setup-axis priorities, lock schedules, readiness, and payload policy.

When a legacy function mixes these responsibilities, keep route admission and strategic choice in Engine. Move printed validation/resolution only after locating the live resolver and the reusable context operations it needs.

## Registered-card inventory

The active registry contains 34 definitions after the Team Yell's Cheer, Gladion, Guzma, and Erika's Invitation metadata migrations. The source of truth is `src/cards/card_registry.hpp`; this list is a planning index rather than a second registry.

- Pokémon: Appletun (`sv8-140`), Mawile-GX (`sm11-141`), Oricorio (`sm2-55`), Regidrago V (`swsh12-135`). Sources: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/sm11-141 https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/swsh12-135
- Energy: Double Dragon Energy (`xy6-97`). Source: https://api.pokemontcg.io/v2/cards/xy6-97
- Trainers: Arven, Battle VIP Pass, Brilliant Blender, Chaotic Swell, Channeler, Crispin, Dawn, Erika's Invitation, Evolution Incense, Field Blower, Forest of Vitality, Forest Seal Stone, Gladion, Guzma, Guzma & Hala, Hisuian Heavy Ball, Klara, Lusamine, Mysterious Treasure, Pokémon Communication, Powerglass, Professor Burnet, Professor Turo's Scenario, Professor's Letter, Quick Ball, Roseanne's Backup, Secret Box, Team Yell's Cheer, Wishful Baton. Registry with exact-print URLs: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

Do not create a parallel migration for a card whose enhancement or bug branch is already owned. Migration ownership must be checked before selecting the next card.

### Erika's Invitation migration

- Enhancement owner: https://github.com/FlareZ123/pokemon-sims/issues/3598
- Exact Pokémon 151 print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- `src/cards/trainers/erikas_invitation.hpp` and the explicit registry own identity, canonical print ID, display name, Trainer kind, and Supporter subtype.
- `name()` and `is_supporter()` delegate those intrinsic facts through the registry instead of duplicating Erika's Invitation in legacy switches.
- Focused registration coverage: `tests/erikas_invitation_card_class_tests.cpp`.
- Preserve opponent-hand inspection, Basic-Pokémon selection, Bench placement, switching, setup-only route policy, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 state, and lock handling at their current Engine owners.
- Any future printed-resolution migration must preserve the printed reveal, optional Basic placement, and conditional switch sequence through general rules operations while leaving target choice and route admission outside card code. Exact printed effect: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Supporter procedure and one-Supporter-per-turn rule: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

### Guzma migration

- Enhancement owner: https://github.com/FlareZ123/pokemon-sims/issues/3618
- Exact Burning Shadows print: https://api.pokemontcg.io/v2/cards/sm3-115
- `src/cards/trainers/guzma.hpp` and the explicit registry own identity, canonical print ID, display name, Trainer kind, and Supporter subtype.
- `name()` and `is_supporter()` delegate those intrinsic facts through the registry rather than duplicating Guzma in legacy switches.
- Focused registration coverage: `tests/guzma_card_class_tests.cpp`.
- Preserve switching sequence, Active/Bench selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 state, locks, and route strategy at their current Engine owners.
- Any future printed-resolution migration must preserve Guzma's opponent-switch-then-self-switch procedure through general rules operations while leaving strategic target selection and route admission outside card code. Exact printed effect: https://api.pokemontcg.io/v2/cards/sm3-115
- Supporter procedure and one-Supporter-per-turn rule: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

### Gladion migration

- Enhancement owner: https://github.com/FlareZ123/pokemon-sims/issues/3604
- Exact Crimson Invasion print: https://api.pokemontcg.io/v2/cards/sm4-95
- `src/cards/trainers/gladion.hpp` and the explicit registry own identity, canonical print ID, display name, Trainer kind, and Supporter subtype.
- `name()` and `is_supporter()` delegate those intrinsic facts through the registry rather than duplicating Gladion in legacy switches.
- Focused registration coverage: `tests/gladion_card_class_tests.cpp`.
- Preserve Prize inspection/exchange, K0/K1 knowledge transitions, DCI/UDP/AMR, Supporter contention, target selection, connector routes, and strict-JIT policy at their current Engine owners.
- Any future printed-resolution migration must preserve Gladion's face-down Prize inspection and exchange procedure through a general rules seam while leaving strategic Prize choice and route admission outside card code. Exact printed effect: https://api.pokemontcg.io/v2/cards/sm4-95
- Supporter procedure and one-Supporter-per-turn rule: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

### Team Yell's Cheer migration

- Enhancement owner: https://github.com/FlareZ123/pokemon-sims/issues/3620
- Exact Brilliant Stars print: https://api.pokemontcg.io/v2/cards/swsh9-149
- `src/cards/trainers/team_yells_cheer.hpp` and the explicit registry own identity, canonical print ID, display name, Trainer kind, and Supporter subtype.
- `name()` and `is_supporter()` now delegate those intrinsic facts through the registry rather than duplicating Team Yell's Cheer in legacy switches.
- Focused registration coverage: `tests/team_yells_cheer_card_class_tests.cpp`.
- Preserve the existing discard-recovery resolver, recovery target choice, Supporter contention, DCI/UDP/AMR, connector domination, K0/K1 state, lock handling, and route strategy at their current Engine owners.
- Any future printed-resolution migration must preserve the printed “up to 3” recovery and shuffle ordering through `CardContext` while leaving strategic recovery selection outside card code. Exact printed effect: https://api.pokemontcg.io/v2/cards/swsh9-149
- Supporter procedure and one-Supporter-per-turn rule: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim `Enhancement: migrate <Card Name> to card class architecture` only when unowned.
3. Map every `Card::<Name>` occurrence into metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add exactly one card module and register it explicitly.
5. Move intrinsic metadata/classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration reveals gameplay behavior that is wrong, route that correction through the normal bug-confirmation workflow instead of silently combining it with architecture cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical Engine composition owner. `composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation. `composition/post_014a_overrides.inc` owns late-search composition.

For mechanical `.inc` cleanup:

- merge a composition-only forwarder into its single owner only after proving the receiving member boundary;
- preserve `#define` / `#include` / `#undef` order exactly;
- keep entry and exit macro guards adjacent to moved blocks;
- never move an include across a declaration-order dependency merely to reduce file count;
- keep route admission/projection/decision policy under `src/trace_engine_v2/core/routes/`;
- retain historical `part_*.inc` forwarders only while a live parent include still depends on them;
- keep the Quick Ball base/tail bridge while it marks a real member-declaration boundary;
- validate strict compilation, the regression suite, source-bound traces, and the T2/T3 matrix after composition changes.

C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Root source-contract shims

`src/trace_engine_v2/composition/engine_body.inc` remains the executable owner of `core/card_catalog.inc` and `core/card_classification.inc`. Root `part_000.inc` and `part_001.inc` remain source-contract shims because unified-test generation, raw-source payload contracts, and same-repository documentation anchors still depend on those historical paths.

Canonical catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
Canonical classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
Unified test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

Future retirement of either shim requires migrating every raw-source reader and same-repository line anchor first.

## Forretress cleanup plan

`src/trace_engine_v2/core/forretress/contract.inc` is the canonical Engine-member declaration owner. Runtime definitions live in `src/trace_engine_v2/core/forretress/runtime.inc`. The declaration/runtime split remains required while Engine is a textual class body and runtime definitions are emitted after Engine closes. The declarations are grouped in runtime order by combo setup, Exploding Energy resolution, and search connectors so future moves can preserve each ownership boundary without reconstructing the monolithic sequence.

`src/trace_engine_v2/part_forretress_ex_combo.inc` owns the Garbodor scenario extension directly beside the `core/forretress/runtime.inc` include. Preserve the local `ScenarioExtension` value ownership and the same `std::optional<Scenario>` lookup shape as the public registry.

`src/trace_engine_v2/core/board_state_policy.inc` owns the shared Active-first mutable/const board traversal, prior-turn evolution timing predicate, canonical `BoardIndex` / `OptionalBoardIndex` vocabulary, the shared `0 == Active` / `Bench index + 1` mapping, and exact-pointer `board_index_of()` lookup. `src/trace_engine_v2/core/forretress/contract.inc` consumes that vocabulary rather than redeclaring it, while `src/trace_engine_v2/core/forretress/runtime.inc` continues to own Exploding Energy strategy and resolution. This cleanup changes structure only; DCI/UDP/AMR, K0/K1, connector, readiness, Ability, and evolution policy remain at their prior owners.

Next mechanical Forretress step: migrate Exploding Energy's remaining Active-versus-Bench source/target scans in `core/forretress/runtime.inc` through `board_pokemon_at_index()`, `board_index_matching()`, and `board_index_of()`. Preserve selection, attachment distribution, self-Knock-Out, promotion, and route policy at the Forretress owner.

Board query owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
Forretress runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc
Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
Core evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

## Steven route package cleanup

`src/trace_engine_v2/core/routes/steven_package_policy.inc` owns the issue-1745, issue-1771, issue-1772, and issue-2622 Steven package implementations through colocated route files under `core/routes/`.

The issue-3653 Steven free-slot connector has canonical policy ownership at `src/trace_engine_v2/core/routes/steven_free_slot_connector_policy.inc`, and `composition/opening_engine_overrides.inc` now composes that canonical route directly inside the established `play_steven` macro boundary. The historical `part_issue_3653_steven_free_slot_connector.inc` forwarder is retired, leaving one executable include path for this policy. The live connector predicate is named by its policy intent, `free_blender_preserves_paid_quick_ball`, while #3653 remains the behavior/source anchor. Connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_free_slot_connector_policy.inc Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3653 C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Next mechanical Steven step: inventory the remaining Steven-named root `part_*.inc` files and retire only composition-only forwarders whose canonical `core/routes/` owner can be included at the exact same member and macro boundary. Preserve route admission, DCI/UDP/AMR, Supporter contention, connector domination, and all direct card/rule URLs while doing those moves. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Decision priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

Route specifications: https://github.com/FlareZ123/pokemon-sims/issues/1745 https://github.com/FlareZ123/pokemon-sims/issues/1771 https://github.com/FlareZ123/pokemon-sims/issues/1772 https://github.com/FlareZ123/pokemon-sims/issues/2622 https://github.com/FlareZ123/pokemon-sims/issues/3653

## Shared policy cleanup plan

`src/trace_engine_v2/core/payload_hand_policy.inc` owns payload preference order, payload-zone membership, payload-zone counts, and physical-order first-payload selection through `PayloadPreferencePolicy`. Route files should reuse `payload_zone_contains()` and `first_payload_card_in_zone()` where their existing semantics match.

Keep payload role, DCI/UDP, strict-JIT admission, connector priority, and K0/K1 decisions in Engine strategy. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

`src/trace_engine_v2/core/garbodor_lock_policy.inc` owns Garbodor scenario-prefix matching and seat-relative activation timing. Preserve Garbotoxin semantics and Rule Box lock interaction at their current policy owners. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57

`src/trace_engine_v2/core/routes/oricorio_connector_policy.inc` owns the missing Grass/Fire Vital Dance preflight directly through `needed_oricorio_energy_might_be_in_deck()`. Keep K0/K1 visibility and connector admission there instead of adding another one-line energy wrapper. Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55 Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

Future policy cleanup should reuse these named seams before adding another payload preference loop, board-index scan, Garbodor scenario-label/timing branch, or Oricorio missing-Energy wrapper.

## Route predicate consolidation

Keep route predicates named when a single policy question is repeated or would otherwise be buried in a resolver. `quick_ball_has_live_connector_route()` owns the issue-1016 Quick Ball connector test, while `issue_1674_turo_promotion_ready()` owns the deadline Turo promotion readiness test. Both remain Engine strategy predicates because they depend on route state, readiness, DCI/JIT timing, and connector value. Their callers continue to own the actual Legacy Star and Professor Turo resolution paths.

Future cleanup should prefer one named predicate for an established policy question before adding another inline conjunction in a resolver. Preserve the exact card/rule URLs beside the resolver that consumes the predicate, and keep printed card validation in card/rules code rather than moving route policy into `src/cards/`.

Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
Decision priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

## Numbered policy-fragment migration

Two compatibility forwarders remain for previously anonymous trace-engine fragments that now have named canonical owners at the same textual include boundaries:

- `part_013.inc` forwards to `core/supporter_legacy_runtime.inc`.
- `part_014b.inc` forwards to `core/recovery_supporter_policy.inc`.

`part_013.inc` is still a live composition boundary: `composition/opening_engine_overrides.inc` includes it directly before the opening override continuation. A cleanup CI probe that removed the shim failed both Release and sanitizer compilation at that include, so future retirement must first retarget the parent include to `core/supporter_legacy_runtime.inc` at the identical member boundary. Build evidence: https://github.com/FlareZ123/pokemon-sims/actions/runs/31849652554

`part_014a.inc` is retired. `composition/engine_body.inc` now includes `turn_action_policy_runtime.inc` directly at the exact former `part_014a.inc` member and macro boundary, leaving one executable include path for that runtime owner. The runtime remains at the trace-engine root because its established nested `core/routes/...` includes are relative to that directory.

Future retirement of the remaining forwarders must first migrate raw-source readers and same-repository anchors, then retarget their composition owners at identical member and macro boundaries. Preserve DCI/UDP/AMR behavior, Supporter contention, connector domination, K0/K1 semantics, declaration order, macro order, and direct rule/card URLs throughout that work.

Supporter legacy runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/supporter_legacy_runtime.inc
Turn action runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/turn_action_policy_runtime.inc
Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc
C++ textual-include semantics: https://eel.is/c++draft/cpp.include
Advanced rules and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Decision priorities and knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

## Rules and policy anchors

Advanced rules procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

K0/K1, DCI/JIT, route priority, and lock-model policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy

Exact migrated-card metadata and source URLs: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

## Validation gate

A cleanup PR is mergeable only when:

- strict Release compilation succeeds;
- focused card tests and the full regression suite show no new failure;
- sanitizer/structural checks show no new failure;
- representative `--simulate-this` traces preserve legal action ordering and readiness;
- the paired T2/T3 probability matrix has no unexplained drift;
- the PR contains no unrelated gameplay behavior change.

Known baseline failures must be identified by their existing issue and shown unchanged before merge. Any new gameplay defect discovered during cleanup must go through the separate bug-confirmation workflow.