# Card Class Cleanup

This file is the current-state architecture and migration plan. Historical cleanup-wave notes remain available in Git history. Keep new entries focused on live ownership, remaining work, and validation requirements.

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

The active registry currently contains 30 definitions. The source of truth is `src/cards/card_registry.hpp`; this list is a planning index rather than a second registry.

- Pokémon: Appletun (`sv8-140`), Mawile-GX (`sm11-141`), Oricorio (`sm2-55`), Regidrago V (`swsh12-135`). Sources: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/sm11-141 https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/swsh12-135
- Energy: Double Dragon Energy (`xy6-97`). Source: https://api.pokemontcg.io/v2/cards/xy6-97
- Trainers: Arven, Battle VIP Pass, Brilliant Blender, Chaotic Swell, Channeler, Crispin, Dawn, Evolution Incense, Field Blower, Forest of Vitality, Forest Seal Stone, Guzma & Hala, Hisuian Heavy Ball, Klara, Lusamine, Mysterious Treasure, Pokémon Communication, Powerglass, Professor Burnet, Professor Turo's Scenario, Professor's Letter, Quick Ball, Roseanne's Backup, Secret Box, Wishful Baton. Registry with exact-print URLs: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

Do not create a parallel migration for a card whose enhancement or bug branch is already owned. Migration ownership must be checked before selecting the next card.

### Oricorio migration

- Enhancement owner: https://github.com/FlareZ123/pokemon-sims/issues/3712
- Exact Guardians Rising print: https://api.pokemontcg.io/v2/cards/sm2-55
- `src/cards/pokemon/oricorio.hpp` and the explicit registry own intrinsic identity, Basic stage, Psychic type, printed Retreat Cost 1, and no-Rule-Box metadata.
- Preserve the established simulator display label `Oricorio GRI 55` because readable seeded trace contracts depend on it: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_2310_turo_oricorio_trace_order_tests.cpp
- Vital Dance resolution, Energy target choice, DCI/UDP/AMR, connector domination, K0/K1 state, locks, and route priority remain in Engine. Existing connector owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/oricorio_connector_policy.inc

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

`src/trace_engine_v2/composition/engine_body.inc` remains the executable owner of `core/card_catalog.inc` and `core/card_classification.inc`. Root `part_000.inc` and `part_001.inc` remain source-contract shims because unified-test generation, raw-source payload contracts, and same-repository documentation anchors still depend on those historical paths. Canonical catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Canonical classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc Unified test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py

`part_001.inc` keeps the payload predicate as an explicitly non-executable block comment for raw-source inspection, preserving a single executable implementation. `part_000.inc` keeps its legacy line-anchor range. Future retirement of either shim requires migrating every raw-source reader and same-repository line anchor first. Validate strict compilation, full regression and sanitizer suites, permanent `--simulate-this` audits, and the paired T2/T3 matrix before retirement. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Forretress cleanup plan

`src/trace_engine_v2/core/forretress/contract.inc` is the canonical Engine-member declaration owner. Runtime definitions live in `src/trace_engine_v2/core/forretress/runtime.inc`. The declaration/runtime split remains required while Engine is a textual class body and runtime definitions are emitted after Engine closes. Contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/contract.inc

`src/trace_engine_v2/part_forretress_ex_combo.inc` owns the Garbodor scenario extension directly beside the `core/forretress/runtime.inc` include. Its append and label-lookup behavior now share one local `ScenarioExtension` abstraction, keeping scenario ordering and lookup ownership together without introducing another `.inc` file. Scenario owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo.inc Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142 Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808

`src/trace_engine_v2/core/board_state_policy.inc` routes mutable and const board queries through one Active-first zone traversal implementation. Keep this shared traversal as the board-query seam. C++ `find_if` semantics: https://eel.is/c++draft/alg.find

The Forretress runtime now uses the contract's `BoardIndex`, `OptionalBoardIndex`, and `AttachmentDestinations` vocabulary for board-index lookup, Exploding Energy source tracking, and attachment destinations. This is a type-only ownership cleanup: Exploding Energy's legal selection range, search, attachment distribution, shuffle, self-Knock-Out, promotion, DCI/UDP/AMR, connector domination, K0/K1 timing, and readiness policy remain at their existing owners. Current runtime: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

The board policy now centralizes ordinary evolution turn timing and exposes `find_evolvable_pineco(...)` through the canonical Active-first traversal. That seam preserves Forest of Vitality as an explicit caller-supplied exception while keeping the card-specific Grass evolution restriction visible. Board-query owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117

Next mechanical Forretress step: replace the local Active/Bench Pineco scan inside `evolve_forretress_ex()` with `find_evolvable_pineco(...)` when declaration order permits the runtime to consume the new seam. Keep that edit traversal-only and preserve Forest of Vitality's established evolution timing. Runtime owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/runtime.inc

Pineco / Forretress ex exact cards: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2

## Steven route package cleanup

`src/trace_engine_v2/core/routes/steven_package_policy.inc` now owns the issue-1745, issue-1771, issue-1772, and issue-2622 Steven package implementations through colocated route files under `core/routes/`. The historical root implementation files for all four routes are retired while preserving their established include order and existing direct rule/card citations. Package owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_package_policy.inc Route specifications: https://github.com/FlareZ123/pokemon-sims/issues/1745 https://github.com/FlareZ123/pokemon-sims/issues/1771 https://github.com/FlareZ123/pokemon-sims/issues/1772 https://github.com/FlareZ123/pokemon-sims/issues/2622

The issue-3653 Steven free-slot connector now has canonical policy ownership at `src/trace_engine_v2/core/routes/steven_free_slot_connector_policy.inc`. The historical `part_issue_3653_steven_free_slot_connector.inc` path is a source-contract forwarder because `composition/opening_engine_overrides.inc` still includes that path inside the existing `play_steven` macro boundary. Connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_free_slot_connector_policy.inc Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3653

Next mechanical Steven step: retire the issue-3653 root forwarder only after the opening composition owner can include the canonical route path at the identical macro boundary and every raw-source consumer of the historical path is migrated. Keep that change behavior-neutral and preserve the direct Steven, Blender, Quick Ball, Regidrago, advanced-manual, and decision-priority URLs with the canonical route policy. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Shared policy cleanup plan

`src/trace_engine_v2/core/payload_hand_policy.inc` owns payload preference order and payload-zone scans through `PayloadPreferencePolicy`. Keep payload role, DCI/UDP, strict-JIT admission, and connector priority in Engine strategy. Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment

`src/trace_engine_v2/core/garbodor_lock_policy.inc` owns Garbodor scenario recognition and activation timing through `GarbodorScenarioPolicy`. Garbotoxin semantics, shared lock-removal state, and Rule Box lock interaction remain at their existing policy owners. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142 Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808

Future policy cleanup should reuse these named seams before adding another payload preference loop or Garbodor scenario-label/timing branch. Keep behavior changes on separately confirmed bug branches.

## Rules and policy anchors

Advanced rules procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

K0/K1, DCI/JIT, route priority, and lock-model policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

Exact migrated-card metadata and source URLs: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

## Validation gate

A cleanup PR is mergeable only when:

- strict Release compilation succeeds;
- focused card tests and the full regression suite show no new failure;
- sanitizer/structural checks show no new failure;
- representative `--simulate-this` traces preserve legal action ordering and readiness;
- the paired T2/T3 probability matrix has no unexplained drift;
- the PR contains no unrelated card migration or gameplay behavior change.

Known baseline failures must be identified by their existing issue and shown unchanged before merge. Any new gameplay defect discovered during cleanup must follow the bug-confirmation workflow.
