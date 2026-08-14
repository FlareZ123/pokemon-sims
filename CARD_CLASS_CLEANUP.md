# Card Class Cleanup

**Cleanup directive:** each cleanup wave selects exactly one card that is already modeled by the simulator and has not yet entered the card-class architecture. Search for an existing migration issue, file and claim an enhancement when none exists, and keep the migration behavior-preserving.

An **unimplemented card** in this document means a card that is not yet implemented in the new card-class architecture. It may already be fully modeled by the legacy simulator.

## Bootstrap gate

Do not begin another migration unless the Quick Ball reference seam exists:

```text
src/cards/card_id.hpp
src/cards/card_definition.hpp
src/cards/card_registry.hpp
src/cards/trainers/quick_ball.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/card_context_adapter.inc
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball is the reference because it demonstrates explicit registration, exact-print metadata, cost validation, K0 -> K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Card source: https://api.pokemontcg.io/v2/cards/swsh1-179

## Migration ledger

### Quick Ball

- Status: active reference migration complete.
- Module: `src/cards/trainers/quick_ball.hpp`.
- Canonical print: `swsh1-179`.
- Active compatibility bridges: `quick_ball_card_class_base.inc` and `quick_ball_card_class_tail.inc`.
- Keep the split bridges until declaration-order dependencies can be removed without changing gameplay.

### Professor's Letter

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3434
- Canonical print: `xy1-123`.
- Card data: https://api.pokemontcg.io/v2/cards/xy1-123
- Status: exact metadata and intrinsic Item classification are owned by `src/cards/trainers/professors_letter.hpp` and the explicit registry.
- Legacy compatibility cleanup: `name()` no longer duplicates the Professor's Letter display name; registered metadata is the sole name owner.
- Existing strategy remains in Engine, including the Earthen Vessel comparison and Energy-axis route selection. Current ordering: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
- Existing route behavior remains governed by https://github.com/FlareZ123/pokemon-sims/issues/2509
- Follow-up for this card must locate the single live `play_professors_letter()` printed-resolution owner before moving state transitions. Do not duplicate or bypass the active resolver through a second gameplay entry point.

### Evolution Incense

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3471
- Canonical print: `swsh1-163`.
- Card data: https://api.pokemontcg.io/v2/cards/swsh1-163
- Status: exact identity, display name, Trainer kind, and Item subtype are owned by `src/cards/trainers/evolution_incense.hpp` and `kRegisteredCardDefinitions`.
- Existing Evolution Incense strategy and resolution remain in Engine for this wave. K0/K1 search timing, DCI/UDP/AMR, connector domination, route priority, target choice, and readiness behavior remain unchanged.
- Focused registration coverage: `tests/evolution_incense_card_class_tests.cpp`.
- Legacy fallback cases remain compatibility-only and are unreachable for registered Evolution Incense. Their later removal should stay a mechanical legacy-table edit rather than being mixed with gameplay behavior.

### Mysterious Treasure

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3473
- Canonical print: `sm6-113`.
- Card data: https://api.pokemontcg.io/v2/cards/sm6-113
- Status: exact metadata and intrinsic Item classification are owned by `src/cards/trainers/mysterious_treasure.hpp` and `kRegisteredCardDefinitions`.
- Existing Mysterious Treasure strategy and resolution remain in Engine. This cleanup does not alter its one-card discard cost, Psychic-or-Dragon target rule, search/shuffle sequence, DCI admission, connector priority, or K0/K1 timing. Printed effect: https://api.pokemontcg.io/v2/cards/sm6-113
- Follow-up for this card must locate the single live `play_mysterious_treasure()` resolution owner and current target-choice boundary before moving printed resolution. Preserve exact discard, search/reveal, K1, and shuffle ordering through `CardContext` without adding strategy queries to card code.

### Brilliant Blender

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3492
- Canonical print: `sv8-164`.
- Card data: https://api.pokemontcg.io/v2/cards/sv8-164
- Status: exact identity, display name, Trainer kind, Item subtype, and ACE SPEC classification are owned by `src/cards/trainers/brilliant_blender.hpp` and `kRegisteredCardDefinitions`.
- Legacy `name()` and `is_item()` compatibility cases are removed. Registered metadata is the sole display-name and Item-subtype owner; the remaining legacy ACE SPEC query can be consolidated separately without changing gameplay.
- Existing Brilliant Blender strategy and resolution remain in Engine. This migration preserves the printed search-for-up-to-five-Pokémon discard resolution, ACE SPEC scarcity, payload selection, DCI/UDP/AMR, connector domination, K0/K1 timing, and ready-turn policy. Printed effect and ACE SPEC rule: https://api.pokemontcg.io/v2/cards/sv8-164
- Follow-up for this card must locate the single live `play_brilliant_blender()` resolver before moving printed resolution. Keep strategic payload choice in Engine and preserve exact deck inspection, discard, shuffle, and knowledge transitions through `CardContext`.

### Hisuian Heavy Ball

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3498
- Canonical print: `swsh10-146`.
- Card data: https://api.pokemontcg.io/v2/cards/swsh10-146
- Status: exact identity, display name, Trainer kind, and Item subtype are owned by `src/cards/trainers/hisuian_heavy_ball.hpp` and `kRegisteredCardDefinitions`.
- Existing Hisuian Heavy Ball strategy and resolution remain in Engine for this metadata-only wave. Prize inspection, Basic-Pokémon choice, Prize replacement, shuffle, K0/K1 timing, DCI/AMR, connector priority, and readiness behavior remain unchanged.
- Legacy `name()` and `is_item()` compatibility cases are removed. The registered definition is now the sole owner of those intrinsic facts.
- Follow-up for this card must locate the single live Hisuian Heavy Ball resolver before moving printed Prize inspection and replacement through `CardContext`; preserve the printed branch that discards the Item when no Basic Pokémon is revealed. Printed effect: https://api.pokemontcg.io/v2/cards/swsh10-146

### Field Blower

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3512
- Canonical print: `sm2-125`.
- Card data: https://api.pokemontcg.io/v2/cards/sm2-125
- Status: exact identity, display name, Trainer kind, and Item subtype are owned by `src/cards/trainers/field_blower.hpp` and `kRegisteredCardDefinitions`.
- Legacy `name()` and `is_item()` compatibility tables no longer duplicate Field Blower's intrinsic metadata. Existing lock-removal strategy and printed resolution remain in `src/trace_engine_v2/core/forest_field_blower_policy.inc` for this wave.
- Focused registration coverage: `tests/field_blower_card_class_tests.cpp`.
- Follow-up must locate the single live Field Blower printed-resolution owner before moving state transitions. Preserve target choice and all lock-removal policy in Engine until a reusable `CardContext` boundary exists.

These staged entries advance the card-class plan without changing the simulator's DCI, AMR, connector-domination, K0/K1, or ready-turn policy.

### Cleanup wave 2026-08-13 checkpoint

- Registered display names for Battle VIP Pass, Brilliant Blender, Field Blower, Hisuian Heavy Ball, Professor's Letter, Evolution Incense, Mysterious Treasure, Quick Ball, and Guzma & Hala now flow through `CardDefinition`; their legacy `name()` branches no longer duplicate those strings. Canonical registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `is_item()` delegates every registered Item to registry metadata. Its legacy switch now contains only unmigrated Items: Secret Box, Ultra Ball, Pokémon Communication, and Earthen Vessel. Exact migrated Item prints: https://api.pokemontcg.io/v2/cards/swsh8-225 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh10-146 https://api.pokemontcg.io/v2/cards/xy1-123 https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179
- This wave is mechanical ownership cleanup only. Printed resolution, strategy, DCI/UDP/AMR, connector priority, and K0/K1 transitions remain at their existing owners. The next resolver migration must still locate the single live resolution boundary before moving state transitions. Architecture contract: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#card-module-contract

## Composition consolidation status

The canonical Engine composition owner is `src/trace_engine_v2/composition/engine_body.inc`.

`src/trace_engine_v2/composition/post_014a_overrides.inc` owns late-search composition. Former one-purpose late-search wrappers were merged there while preserving textual include order and macro lifetimes.

`src/trace_engine_v2/composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation. The former one-purpose Supporter wrapper was inlined there while preserving the `part_011.inc` -> `part_012.inc` -> `part_013.inc` order.

The issue-962 eligibility, projection, and decision sections now compose directly at their proven class-member boundary in `src/trace_engine_v2/part_014a.inc`. The former `part_014a_issue_962_eligibility.inc`, projection marker, and decision marker are retired. The shared implementation remains solely owned by `src/trace_engine_v2/core/routes/issue_962_route.inc`, with eligibility, projection, and decision included in dependency order. Core route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_962_route.inc

`src/trace_engine_v2/core/crispin_trace_provenance.inc` directly owns the issue-3152 Steven/Secret Box comparator beside its `bench_pineco_if_useful` handoff. The retired comment-only `part_issue_3152_steven_prized_box_override.inc` path was deleted after repository-wide reference checks found no live include dependency. Preserve the existing macro order through `part_issue_1118_secret_box.inc` and its release points in `part_issue_1369_celestial_roar_secret_box_override.inc`. Sources: https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/sv6-163 https://github.com/FlareZ123/pokemon-sims/issues/3152

`src/trace_engine_v2/core/simulation_runtime.inc` now owns the state-adjacent runtime types. The former `core/game_state_types.inc`, `core/simulation_metrics.inc`, and `core/trace_log.inc` one-owner fragments are merged there in their historical declaration order, while `core/simulator_state.inc` keeps the single legacy continuation boundary. C++ textual include semantics: https://eel.is/c++draft/cpp.include

Keep `simulation_runtime.inc` limited to state/runtime data types and trace ownership. Gameplay strategy, DCI/AMR/K0/K1 policy, and card effects remain in Engine, policy, and card modules.

The retired `src/trace_engine_v2/part_late_policy_bundle.inc` comment-only compatibility path was deleted after repository-wide reference checks found no live include dependency. Its former Quick Ball, Crispin provenance, Secret Box, and Celestial Roar chain remains solely owned by `composition/post_014a_overrides.inc`; future cleanup must continue at that canonical owner rather than recreating the historical bundle. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
The former `src/trace_engine_v2/part_issue_989_wonder_tag_complete_route_override.inc` forwarding shim is retired. `composition/opening_engine_overrides.inc` now includes `core/tapu_wonder_tag_route_policy.inc` directly at the same member boundary, preserving the existing macro lifetime while removing one redundant `.inc` layer. Keep Wonder Tag route logic in the core owner. Canonical policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tapu_wonder_tag_route_policy.inc Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60

Registered-card compatibility now uses `find_definition()` as the single registry lookup. `has_definition()` and intrinsic Item classification delegate to that lookup instead of maintaining parallel card switches, reducing duplicate ownership as additional cards migrate.

### Registry consolidation checkpoint

- `kRegisteredCardDefinitions` is the single explicit list of migrated definitions. Battle VIP Pass, Brilliant Blender, Evolution Incense, Field Blower, Guzma & Hala, Hisuian Heavy Ball, Mysterious Treasure, Professor's Letter, and Quick Ball use that inventory; future migrations append one definition there instead of extending a lookup switch. Representative sources: https://api.pokemontcg.io/v2/cards/swsh8-225 https://api.pokemontcg.io/v2/cards/sv8-164 https://api.pokemontcg.io/v2/cards/sm12-229 https://api.pokemontcg.io/v2/cards/swsh10-146
- Registered display-name ownership is now mechanically complete for the current registry inventory: `name()` consults `find_definition()` first and has no duplicate return strings for registered cards.
- Registered Item ownership is now mechanically complete for the current registry inventory: `is_item()` delegates registered cards before reaching a legacy switch that contains only unmigrated Items.
- `registered_is_trainer_kind()` is the shared intrinsic Trainer-subtype query. Item, Supporter, Stadium, and Tool compatibility checks should delegate to this helper as those classifications migrate.
- `is_trainer_kind()` belongs with `CardDefinition` because it interprets intrinsic metadata only. Route policy, DCI/UDP, AMR, connector domination, K0/K1, and matchup state remain outside the registry.
- The next card migration should reuse these registry primitives before adding any new compatibility branch. If a migrated fact still needs a legacy fallback, keep that fallback only for unmigrated cards.

For mechanical `.inc` cleanup:

- merge a composition-only forwarding file into its single owner only after proving the receiving member boundary;
- preserve `#define` / `#include` / `#undef` order exactly;
- keep entry and exit macro guards adjacent to the moved block;
- never move an include across a declaration-order dependency to reduce file count;
- organize route admission, projection, and decision policy owners under `src/trace_engine_v2/core/routes/`; keep historical `part_*.inc` files as thin boundary forwarders only while a live parent include still depends on them;
- do not merge the Quick Ball base/tail bridge while their split marks a real member-declaration boundary;
- retire an obsolete compatibility path to a comment-only marker before deleting it when repository tooling or historical source links still depend on the path;
- validate strict compilation, the regression suite, and representative `--simulate-this` traces after composition changes.

C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Dependency direction

Preserve this direction:

```text
rules <- cards <- simulator/strategy
```

The operating rule is:

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine`/`State` data.

## Architecture contracts

### `card_id.hpp`

`src/cards/card_id.hpp` owns the stable `sim::Card` identifier set. Do not add a second card-ID system or renumber existing enumerators during cleanup. External exact-print identity belongs in `CardDefinition::canonical_id`.

### `card_definition.hpp`

`CardDefinition` stores intrinsic exact-print facts such as name, canonical print id, Trainer subtype, stage/type, retreat cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and a direct source URL.

Do not store Regidrago policy in card metadata. Payload role, DCI, strict-JIT value, AMR, route priority, matchup logic, Supporter contention, and setup-axis value belong in simulator strategy.

### `card_registry.hpp`

Registration is explicit and deterministic. Do not use static-constructor self-registration or linker-retention behavior.

Compatibility code consults registered metadata first, then legacy tables for unmigrated cards. `find_definition()` is the canonical registry lookup over `kRegisteredCardDefinitions`; helper predicates should derive from that definition instead of introducing parallel registration switches. When a migrated intrinsic fact is fully owned by the registry, remove its duplicate legacy case in a later safe mechanical edit.

### `card_context.hpp`

`CardContext` is the narrow reusable rules seam. Add only general game operations needed by printed card effects. Never add card-specific operations such as `resolve_quick_ball()` or route-policy queries such as `can_use_crispin()`.

Knowledge transitions, zone mutations, shuffle behavior, and trace ordering must remain compatible with the existing simulator unless a separately confirmed bug authorizes a behavior change.

## Card module contract

Each migrated card gets one primary module under `src/cards/pokemon/`, `src/cards/trainers/`, or `src/cards/energy/`.

A card module owns:

- exact-print metadata;
- printed action/choice shape;
- intrinsic cost legality;
- printed target categories and cardinality;
- printed effect resolution through `CardContext`;
- direct source identity.

Engine/strategy owns:

- whether the current route wants to play the card;
- DCI/UDP/AMR decisions;
- strict-JIT and matchup-flex timing;
- Supporter contention;
- connector domination and setup-axis priorities;
- scenario lock schedules;
- readiness and payload policy;
- strategic target preference.

When a legacy function mixes these responsibilities, leave route admission and strategic choice in Engine. Move printed validation/resolution only after the live owner and required reusable context operations are known.

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim `Enhancement: migrate <Card Name> to card class architecture` when unowned.
3. Map every `Card::<Name>` occurrence into metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add exactly one card module.
5. Register it explicitly.
6. Move intrinsic metadata/classification ownership first.
7. Route active printed resolution through the card module only after locating the live resolver and preserving K0/K1 timing.
8. Keep strategy in Engine.
9. Add focused tests for exact metadata and printed legality/effect boundaries as each phase becomes active.
10. Run the full CI matrix and representative `--simulate-this` traces before merge.

If migration reveals a gameplay defect, file it through the normal bug workflow. Do not silently combine the behavior correction with architecture cleanup.

## Validation gate

A cleanup PR is mergeable only when:

- strict Release compilation succeeds;
- focused card tests and the full regression suite succeed;
- sanitizer/structural checks have no new failure;
- representative `--simulate-this` traces preserve legal action ordering and readiness;
- the T2/T3 probability matrix has no unexplained drift;
- the PR contains no second card migration.

Rules source for Item/Supporter/search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Policy source for K0/K1, DCI/JIT, route priority, and lock modeling: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

## Cleanup wave 2026-08-13 composition follow-up

- Forest Seal Stone holder selection now has one selector in `src/trace_engine_v2/part_010_attach_fss_override.inc`. The `allow_active` parameter preserves the existing general Active-first attachment order and the Powerglass-specific Bench-only reservation path. Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156 Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
- The issue-3040 Supporter continuation in `src/trace_engine_v2/part_issue_1070_tate_after_vstar_search_override.inc` now exposes a responsibility-based fallback name while preserving the existing macro boundary and Turo route. Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
- The issue-962 eligibility, projection, and decision sections now compose directly in `part_014a.inc` at the same proven member boundary. Their three compatibility `.inc` files are retired, while implementation ownership remains in `core/routes/issue_962_route.inc`.
- The issue-1447 Vessel-hold route now includes its canonical `core/routes/issue_1447_vessel_hold_policy.inc` owner directly from `part_014a.inc`; the forwarding shim is retired. Future route cleanup should preserve this direct-owner pattern when declaration order permits it.

## Battle VIP Pass metadata migration

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3509
- Canonical print: `swsh8-225`; exact card data: https://api.pokemontcg.io/v2/cards/swsh8-225
- `src/cards/trainers/battle_vip_pass.hpp` and `kRegisteredCardDefinitions` own exact identity, display name, Trainer kind, and Item subtype.
- Focused registration coverage: `tests/battle_vip_pass_card_class_tests.cpp`.
- Existing first-turn admission, Basic-Pokémon Bench search resolution, bench-space policy, K0/K1 timing, DCI/UDP/AMR, connector domination, and route ordering remain in Engine. A later printed-resolution migration should preserve those boundaries and reuse the existing live resolver.

## Cleanup wave 2026-08-13 issue-962 and route-policy plan

- Completed: `part_014a.inc` directly composes issue-962 eligibility, projection, and decision from `core/routes/issue_962_route.inc` in dependency order. The former eligibility composition shim and empty projection/decision markers are retired. Shared route implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_962_route.inc
- Route fragments that contain policy lambdas but no composition aliases belong under `src/trace_engine_v2/core/routes/`. A temporary forwarding include is appropriate only while a live parent include still depends on it. Once the parent boundary is proven, include the canonical owner directly and retire the forwarder. C++ include semantics: https://eel.is/c++draft/cpp.include
- Completed: `part_014a.inc` directly includes `core/routes/issue_1447_vessel_hold_policy.inc`, and `part_014a_issue_1447_vessel_hold.inc` is retired. The policy blob and its existing rule, card, policy, and issue URLs remain unchanged. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_1447_vessel_hold_policy.inc
- The shared board-state seam now keeps Active-first predicate short-circuiting explicit in `core/board_state_policy.inc`, while `core/forretress_combo_contract.inc` centralizes its board index and attachment-destination types. Keep future Forretress composition cleanup on those named seams rather than reintroducing raw index container types in forwarding `.inc` files. Board policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc Forretress contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress_combo_contract.inc

## Cleanup wave 2026-08-13 Crobat CLI consolidation

- `src/trace_engine_v2/cli/crobat_modeling.inc` now directly owns temporary Crobat deck construction, lookup, simulation scheduling, and CSV reporting inside its single `sim` namespace boundary. The former `crobat_modeling_decks.inc` and `crobat_modeling_matrix.inc` one-owner fragments were folded into that owner in declaration order and retired.
- Keep future Crobat-modeling organization inside `crobat_modeling.inc` unless a reusable cross-CLI abstraction emerges. Do not recreate forwarding `.inc` files solely to separate adjacent declarations. Modeling contract: https://github.com/FlareZ123/pokemon-sims/issues/1394
- This consolidation is behavior-preserving: recipe validation, common-random-number seed slots, atomic CSV writes, and the existing Crobat modeling command contract remain unchanged. Atomic result-write contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#windows-build-and-result-write-behavior

## Powerglass metadata migration

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3553
- Canonical print: `sv6pt5-63`; exact card data: https://api.pokemontcg.io/v2/cards/sv6pt5-63
- `src/cards/trainers/powerglass.hpp` and `kRegisteredCardDefinitions` now own Powerglass identity, display name, Trainer kind, and Pokémon Tool subtype.
- Legacy `name()` and `is_tool()` compatibility paths delegate registered Powerglass metadata to the registry while retaining fallbacks for unmigrated Tools. Focused coverage: `tests/powerglass_card_class_tests.cpp`.
- Existing end-of-turn Basic Energy attachment resolution, Active-position requirement, attachment-destination strategy, DCI/UDP/AMR, K0/K1, connector domination, and setup-axis policy remain in Engine. A later resolver migration must first locate the single live Powerglass resolution boundary and preserve end-of-turn sequencing through `CardContext`.

## Chaotic Swell metadata migration

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3563
- Canonical print: `sm12-187`; exact card data: https://api.pokemontcg.io/v2/cards/sm12-187
- `src/cards/trainers/chaotic_swell.hpp` and `kRegisteredCardDefinitions` own Chaotic Swell identity, display name, Trainer kind, and Stadium subtype.
- Legacy `name()` and `is_stadium()` compatibility paths delegate registered Chaotic Swell metadata to the registry while retaining Stadium fallbacks only for unmigrated cards. Focused coverage: `tests/chaotic_swell_card_class_tests.cpp`.
- Existing Stadium placement and replacement behavior, opponent-Stadium cancellation, strategy, DCI/UDP/AMR, K0/K1, connector domination, and lock interactions remain in Engine. A later resolver migration must locate the single live Chaotic Swell effect owner before moving printed Stadium cancellation through `CardContext`. Printed effect: https://api.pokemontcg.io/v2/cards/sm12-187

## Dawn metadata migration

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3560
- Canonical print: `me2-87`; exact card data: https://api.pokemontcg.io/v2/cards/me2-87
- `src/cards/trainers/dawn.hpp` and `kRegisteredCardDefinitions` now own Dawn identity, display name, Trainer kind, and Supporter subtype.
- Legacy `name()` and `is_supporter()` compatibility tables no longer duplicate Dawn's intrinsic facts; both now reach the registered definition before their unmigrated fallbacks. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- Focused registration coverage: `tests/dawn_card_class_tests.cpp`.
- This wave intentionally leaves Dawn's printed Basic/Stage 1/Stage 2 deck search, reveal, hand movement, shuffle, Supporter contention, K0/K1 transitions, DCI/UDP/AMR, connector domination, and route strategy at their current Engine owners. Printed effect: https://api.pokemontcg.io/v2/cards/me2-87
- A later resolver migration must first locate the single live Dawn resolution boundary and preserve the printed search/reveal/shuffle order through `CardContext` without moving strategic target preference into card code.

## Cleanup wave 2026-08-13 board and Forretress contract follow-up

- `src/trace_engine_v2/core/board_state_policy.inc` now owns one Active-first `find_board_pokemon_matching()` traversal. Boolean board queries delegate to that finder, preserving the repository's explicit Active-before-Bench short-circuit contract while giving later cleanup one reusable board lookup seam. C++ algorithm semantics: https://eel.is/c++draft/alg.find
- `src/trace_engine_v2/core/forretress_combo_contract.inc` now names `OptionalBoardIndex` beside `BoardIndex` and `AttachmentDestinations`, and marks pure combo queries `[[nodiscard]]`. This keeps board-index/result vocabulary in the declared contract rather than spreading raw container types through future forwarding `.inc` files. Pineco / Forretress ex sources: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
- Next safe follow-up: migrate the out-of-class Forretress definitions to the existing `BoardIndex`, `OptionalBoardIndex`, and `AttachmentDestinations` aliases only after the declaration boundary is proven in CI. Keep that future edit type-only, preserve Exploding Energy's existing card/ruling URLs beside its resolver, and do not combine it with route-policy changes. Current implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_forretress_ex_combo_legacy.inc

## Cleanup wave 2026-08-13 late-Supporter route consolidation

- Completed: the issue-3040 Professor Turo staging implementation now lives at `src/trace_engine_v2/core/routes/professor_turo_regidrago_staging_policy.inc`; the former root-level `part_issue_3040_turo_regidrago_staging_override.inc` fragment is retired. Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/professor_turo_regidrago_staging_policy.inc Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
- The issue-1070 late-Supporter selector composition now lives beside that route at `src/trace_engine_v2/core/routes/tate_after_vstar_search_selector.inc`. Its historical `part_issue_1070_tate_after_vstar_search_override.inc` path is intentionally a thin boundary forwarder while `composition/post_014a_overrides.inc` still consumes that compatibility name. Canonical selector: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/tate_after_vstar_search_selector.inc
- Preserve the existing `choose_supporter` macro lifetime and the selector's final-fallback ordering. A later cleanup may include the canonical selector directly from `composition/post_014a_overrides.inc` and retire the remaining forwarder only after that class-member boundary is revalidated. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Crispin metadata migration

- Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3580
- Canonical print: `sv7-133`; exact card data: https://api.pokemontcg.io/v2/cards/sv7-133
- `src/cards/trainers/crispin.hpp` and `kRegisteredCardDefinitions` now own Crispin identity, display name, Trainer kind, and Supporter subtype.
- Legacy `name()` and `is_supporter()` compatibility tables no longer duplicate Crispin's intrinsic facts; registered metadata is the sole owner. Supporter/search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
- Focused registration coverage: `tests/crispin_card_class_tests.cpp`.
- This wave intentionally leaves Crispin's printed search for up to two Basic Energy cards of different types, one-to-hand/one-attach resolution, reveal and shuffle ordering, Supporter contention, K0/K1 transitions, DCI/UDP/AMR, connector domination, and route strategy at their current Engine owners. Printed effect: https://api.pokemontcg.io/v2/cards/sv7-133
- A later resolver migration must first locate the single live Crispin resolution boundary and preserve printed search/reveal/attachment/hand/shuffle ordering through `CardContext` without moving strategic Energy choice into card code.

## Cleanup wave 2026-08-14 Arven post-search consolidation

- `src/trace_engine_v2/composition/post_014a_overrides.inc` now owns the Powerglass Arven continuation directly at the existing `play_arven_fss_blender_contention_original` macro boundary. The former `part_012_powerglass_override.inc` fragment is retired. Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63 Arven: https://api.pokemontcg.io/v2/cards/sv1-166 C++ textual include semantics: https://eel.is/c++draft/cpp.include
- The same post-search owner now contains the Garbodor/Field Blower Arven continuation immediately after the empty-deck Arven alias release. The former `part_arven_garbodor_field_blower_override.inc` fragment is retired. Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125 Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Existing route specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
- This consolidation is mechanical. It preserves the exact function bodies, macro lifetimes, helper include, direct card/rule URLs, and declaration order. Future Arven cleanup should continue from `composition/post_014a_overrides.inc` and avoid recreating one-owner `part_*.inc` fragments.
- Validation gate remains strict Release compilation, the full regression suite, representative `--simulate-this` traces, and the paired T2/T3 matrix. Any matrix movement requires a separately justified behavior change rather than being accepted as cleanup drift.

## Cleanup wave 2026-08-14 late-Supporter owner merge

- Completed: `src/trace_engine_v2/core/routes/tate_after_vstar_search_selector.inc` now owns both the issue-3040 Professor Turo staging policy and the issue-1070 late-Supporter fallback selector. The one-function `core/routes/professor_turo_regidrago_staging_policy.inc` fragment is retired, removing a composition layer while preserving the exact function body, selector order, and direct rule/card/issue URLs. Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171 Confirmed route specification: https://github.com/FlareZ123/pokemon-sims/issues/3040
- Keep the historical `part_issue_1070_tate_after_vstar_search_override.inc` path as a thin boundary forwarder while `composition/post_014a_overrides.inc` still consumes that compatibility name. The next safe cleanup is to include the canonical selector directly from `post_014a_overrides.inc` only after the class-member boundary and `choose_supporter` macro lifetime are revalidated. C++ textual-include semantics: https://eel.is/c++draft/cpp.include
- This merge is behavior-preserving. It does not change Supporter legality, Turo resolution, Active replacement, K0/K1, DCI/JIT policy, route priority, or readiness semantics. Validation remains strict Release compilation, full regressions, representative `--simulate-this` traces, and the paired T2/T3 matrix. Rules procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Policy contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md