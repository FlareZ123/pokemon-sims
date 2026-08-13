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

This staged entry advances the card-class plan without changing the simulator's DCI, AMR, connector-domination, K0/K1, or ready-turn policy.

## Composition consolidation status

The canonical Engine composition owner is `src/trace_engine_v2/composition/engine_body.inc`.

`src/trace_engine_v2/composition/post_014a_overrides.inc` owns late-search composition. Former one-purpose late-search wrappers were merged there while preserving textual include order and macro lifetimes.

`src/trace_engine_v2/composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation. The former one-purpose Supporter wrapper was inlined there while preserving the `part_011.inc` -> `part_012.inc` -> `part_013.inc` order.

The unused `composition/issue_962_route_sections.inc` selector was removed after the canonical search stage had already moved to direct issue-962 section includes. The active issue-962 eligibility, projection, and decision sections now compose in dependency order through `src/trace_engine_v2/part_014a_issue_962_eligibility.inc`; the projection and decision paths are compatibility include markers until the historical `part_014a.inc` include sites can be collapsed in a later boundary-safe edit. Core route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/issue_962_route.inc

`src/trace_engine_v2/core/crispin_trace_provenance.inc` directly owns the issue-3152 Steven/Secret Box comparator beside its `bench_pineco_if_useful` handoff. The retired comment-only `part_issue_3152_steven_prized_box_override.inc` path was deleted after repository-wide reference checks found no live include dependency. Preserve the existing macro order through `part_issue_1118_secret_box.inc` and its release points in `part_issue_1369_celestial_roar_secret_box_override.inc`. Sources: https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/sv6-163 https://github.com/FlareZ123/pokemon-sims/issues/3152

`src/trace_engine_v2/core/simulation_runtime.inc` now owns the state-adjacent runtime types. The former `core/game_state_types.inc`, `core/simulation_metrics.inc`, and `core/trace_log.inc` one-owner fragments are merged there in their historical declaration order, while `core/simulator_state.inc` keeps the single legacy continuation boundary. C++ textual include semantics: https://eel.is/c++draft/cpp.include

Keep `simulation_runtime.inc` limited to state/runtime data types and trace ownership. Gameplay strategy, DCI/AMR/K0/K1 policy, and card effects remain in Engine, policy, and card modules.

The retired `src/trace_engine_v2/part_late_policy_bundle.inc` comment-only compatibility path was deleted after repository-wide reference checks found no live include dependency. Its former Quick Ball, Crispin provenance, Secret Box, and Celestial Roar chain remains solely owned by `composition/post_014a_overrides.inc`; future cleanup must continue at that canonical owner rather than recreating the historical bundle. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

`src/trace_engine_v2/part_issue_989_wonder_tag_complete_route_override.inc` remains a thin historical source-link shim whose only implementation dependency is `core/tapu_wonder_tag_route_policy.inc`. Keep route logic in the core owner rather than duplicating it in the compatibility path. Canonical policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tapu_wonder_tag_route_policy.inc

Registered-card compatibility now uses `find_definition()` as the single registry lookup. `has_definition()` and intrinsic Item classification delegate to that lookup instead of maintaining parallel card switches, reducing duplicate ownership as additional cards migrate.

### Registry consolidation checkpoint

- `kRegisteredCardDefinitions` is the single explicit list of migrated definitions. Future migrations append one definition there instead of extending a lookup switch.
- `registered_is_trainer_kind()` is the shared intrinsic Trainer-subtype query. Item, Supporter, Stadium, and Tool compatibility checks should delegate to this helper as those classifications migrate.
- `is_trainer_kind()` belongs with `CardDefinition` because it interprets intrinsic metadata only. Route policy, DCI/UDP, AMR, connector domination, K0/K1, and matchup state remain outside the registry.
- The next card migration should reuse these registry primitives before adding any new compatibility branch. If a migrated fact still needs a legacy fallback, keep that fallback only for unmigrated cards.

For mechanical `.inc` cleanup:

- merge a composition-only forwarding file into its single owner only after proving the receiving member boundary;
- preserve `#define` / `#include` / `#undef` order exactly;
- keep entry and exit macro guards adjacent to the moved block;
- never move an include across a declaration-order dependency to reduce file count;
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

Compatibility code consults registered metadata first, then legacy tables for unmigrated cards. `find_definition()` is the canonical registry lookup; helper predicates should derive from that definition instead of introducing parallel registration switches. When a migrated intrinsic fact is fully owned by the registry, remove its duplicate legacy case in a later safe mechanical edit.

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
