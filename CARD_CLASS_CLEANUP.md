# Card Class Cleanup

This file is the canonical migration plan for moving already-modeled cards from trace-engine metadata tables into the explicit card-class architecture. Git history preserves completed cleanup-wave detail; this document keeps the active contracts, ownership boundaries, and remaining work concise.

## Cleanup directive

Each cleanup wave selects exactly one card that is already modeled by the simulator and is not yet fully represented in the card-class architecture.

1. Search open issues for an existing migration owner.
2. File or reclaim one enhancement only when no active owner exists.
3. Map every `Card::<Name>` occurrence into intrinsic metadata, printed resolution, strategy, test, or documentation.
4. Add exactly one primary module under `src/cards/`.
5. Register it explicitly in `src/cards/card_registry.hpp`.
6. Move intrinsic metadata and classification first.
7. Remove duplicate legacy metadata only after the registry path is live.
8. Move printed resolution only after the single live resolver and required `CardContext` primitives are proven.
9. Keep strategy, DCI/UDP/AMR, connector domination, K0/K1, lock scheduling, and readiness policy in Engine/policy code.
10. Validate strict compilation, focused tests, full regression, representative `--simulate-this` traces, and the T2/T3 matrix before merge.

If migration reveals gameplay behavior that is wrong, file that behavior through the normal bug workflow instead of silently changing it in cleanup.

## Bootstrap gate

Do not begin another resolver migration unless the Quick Ball reference seam remains intact:

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

Quick Ball demonstrates explicit registration, exact-print metadata, printed cost validation, K0 -> K1 search timing, strategy-owned target choice, source-card movement, failed-search handling, shuffle, and trace compatibility. Card data: https://api.pokemontcg.io/v2/cards/swsh1-179

## Dependency direction

Preserve:

```text
rules <- cards <- simulator/strategy
```

**Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Architecture contracts

### `card_id.hpp`

`src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Do not add a second ID system or renumber existing values. Exact external print identity belongs in `CardDefinition::canonical_id`.

### `card_definition.hpp`

`CardDefinition` owns intrinsic facts such as display name, canonical print id, Trainer subtype, stage/type, retreat cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and direct source URL.

Regidrago-specific policy does not belong in card metadata. Payload role, DCI, strict-JIT value, AMR, route priority, matchup logic, Supporter contention, and setup-axis value stay in simulator strategy.

### `card_registry.hpp`

Registration is explicit and deterministic through `kRegisteredCardDefinitions`. `find_definition()` is the canonical lookup. Classification helpers derive from the registered definition rather than maintaining parallel registration switches.

Compatibility code checks registered metadata first, then legacy fallbacks for genuinely unmigrated cards. Once the registry owns an intrinsic fact, remove its duplicate legacy case in a separate mechanical edit.

### `card_context.hpp`

`CardContext` is the reusable printed-rules seam. Add general game operations only. Do not add card-specific route queries or `Engine` policy access.

Knowledge transitions, zone mutations, shuffle behavior, and trace ordering must stay compatible with the existing simulator unless a separately confirmed bug authorizes a behavior change.

## Current metadata ledger

These cards have explicit registered definitions and should not regain duplicate name/subtype ownership in legacy switches:

- Quick Ball, `swsh1-179`: https://api.pokemontcg.io/v2/cards/swsh1-179
- Professor's Letter, `xy1-123`: https://api.pokemontcg.io/v2/cards/xy1-123
- Evolution Incense, `swsh1-163`: https://api.pokemontcg.io/v2/cards/swsh1-163
- Mysterious Treasure, `sm6-113`: https://api.pokemontcg.io/v2/cards/sm6-113
- Battle VIP Pass, `swsh8-225`: https://api.pokemontcg.io/v2/cards/swsh8-225
- Brilliant Blender, `sv8-164`: https://api.pokemontcg.io/v2/cards/sv8-164
- Hisuian Heavy Ball, `swsh10-146`: https://api.pokemontcg.io/v2/cards/swsh10-146
- Field Blower, `sm2-125`: https://api.pokemontcg.io/v2/cards/sm2-125
- Guzma & Hala, `sm12-229`: https://api.pokemontcg.io/v2/cards/sm12-229
- Powerglass, `sv6pt5-63`: https://api.pokemontcg.io/v2/cards/sv6pt5-63
- Chaotic Swell, `sm12-187`: https://api.pokemontcg.io/v2/cards/sm12-187
- Dawn, `me2-87`: https://api.pokemontcg.io/v2/cards/me2-87
- Arven, `sv1-166`: https://api.pokemontcg.io/v2/cards/sv1-166
- Crispin, `sv7-133`: https://api.pokemontcg.io/v2/cards/sv7-133
- Professor Burnet, `swsh12tg-TG26`: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Earthen Vessel, `sv4-163`: https://api.pokemontcg.io/v2/cards/sv4-163

### Earthen Vessel migration checkpoint

Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3475

`src/cards/trainers/earthen_vessel.hpp` and `kRegisteredCardDefinitions` own Earthen Vessel's exact identity, display name, Trainer kind, and Item subtype. `name()` and `is_item()` no longer duplicate those facts. Focused coverage lives in `tests/earthen_vessel_card_class_tests.cpp`.

This cleanup is metadata-only. The existing Earthen Vessel strategy and printed discard-one/search-up-to-two-Basic-Energy resolution remain at their current Engine owner, preserving DCI/UDP/AMR, connector priority, K0/K1 timing, Supporter contention interactions, and ready-turn policy. Printed effect: https://api.pokemontcg.io/v2/cards/sv4-163

The next Earthen Vessel step must locate the single live resolver before moving printed resolution through `CardContext`. Preserve the printed discard-before-search sequence, Basic-Energy target restriction, deck inspection/K1 transition, hand movement, and shuffle ordering. Rules procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Remaining Item metadata migrations

After Earthen Vessel, legacy `is_item()` should contain only currently unmigrated Item families. Active issues own individual migrations; do not overlap them.

- Secret Box
- Ultra Ball
- Pokémon Communication

Before selecting one, search current issues and branches because this agent-swarm repository frequently has in-flight work.

## Composition consolidation rules

The canonical Engine composition owner is `src/trace_engine_v2/composition/engine_body.inc`. Route-policy implementation owners belong under `src/trace_engine_v2/core/routes/` when declaration order permits.

For `.inc` cleanup:

- merge a composition-only forwarding file into its single owner only after proving the receiving member boundary;
- preserve `#define`, `#include`, and `#undef` order exactly;
- keep entry/exit macro guards adjacent to the moved block;
- never move an include across a declaration-order dependency just to reduce file count;
- keep historical `part_*.inc` forwarders only while a live parent include depends on that compatibility boundary;
- do not merge the Quick Ball base/tail bridge while the split marks a real member-declaration boundary;
- retire obsolete compatibility paths only after repository-wide reference checks;
- keep state/runtime data types separate from gameplay strategy and card effects.

C++ textual-include semantics: https://eel.is/c++draft/cpp.include

## Existing canonical composition seams

- `src/trace_engine_v2/composition/post_014a_overrides.inc` owns late-search composition.
- `src/trace_engine_v2/composition/opening_engine_overrides.inc` owns the early Supporter/VSTAR continuation.
- `src/trace_engine_v2/core/routes/issue_962_route.inc` owns issue-962 route policy.
- `src/trace_engine_v2/core/routes/issue_1447_vessel_hold_policy.inc` owns the issue-1447 Vessel-hold policy.
- `src/trace_engine_v2/core/routes/professor_turo_regidrago_staging_policy.inc` owns Professor Turo staging.
- `src/trace_engine_v2/core/routes/tate_after_vstar_search_selector.inc` owns the late Supporter selector.
- `src/trace_engine_v2/core/simulation_runtime.inc` owns state-adjacent runtime types and trace ownership.
- `src/trace_engine_v2/core/tapu_wonder_tag_route_policy.inc` owns Wonder Tag route policy.

Do not recreate retired one-purpose forwarding layers around these owners.

## Validation gate

A cleanup PR is mergeable only when:

- strict Release compilation succeeds;
- focused card tests and the full regression suite have no new failure;
- sanitizer/structural checks have no new failure;
- representative `--simulate-this` traces preserve legal action ordering and earliest-readiness policy;
- the T2/T3 probability matrix has no unexplained drift;
- no second card migration is included.

Rules source for Item/Supporter/search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Policy source for K0/K1, DCI/JIT, route priority, and lock modeling: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
