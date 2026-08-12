# Card Class Cleanup

**Cleanup directive:** read this file, file and claim an enhancement issue for exactly one card that is already modeled by the simulator but has not yet been migrated into the card-class architecture, then migrate only that card using the architecture and compatibility rules below.

In this document, **unimplemented card** means **not yet implemented in the new card-class architecture**. The card may already be modeled by the legacy simulator. This cleanup task is a behavior-preserving migration task, not permission to add unrelated card behavior or change strategy policy.

## Bootstrap gate

The maintainer will land the architecture skeleton and one complete reference migration before agents use this cleanup workflow. The reference migration is **Quick Ball**.

Before filing an issue or editing code, verify that the following bootstrap files exist:

```text
src/cards/card_id.hpp
src/cards/card_definition.hpp
src/cards/card_registry.hpp
src/cards/trainers/quick_ball.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/card_context_adapter.inc
```

If the skeleton or Quick Ball reference migration is missing, stop. Report that the card-class bootstrap is incomplete. Do not invent a competing class hierarchy, registry mechanism, registration side effect, or CMake library layout during a cleanup task.

## Why the migration is incremental

The current simulator is intentionally composed as one translation unit through `src/regidrago_sim.cpp` and the ordered `.inc` pipeline under `src/trace_engine_v2/composition/`. The legacy card model also crosses textual include boundaries: the `Card` catalog begins in `core/card_catalog.inc`, its name switch continues in `core/card_classification.inc`, and some classification helpers continue into `core/simulator_state.inc`.

The migration must therefore allow migrated and legacy cards to coexist. Cleanup migrations must not attempt to replace the whole composition pipeline, create a new linked library target, rename every `Card` use, or move every card at once.

The target source layout is already shaped so it can become a standalone rules/cards library later. During the incremental phase it remains compatible with the existing single-translation-unit build.

## Target architecture

```text
src/
  cards/
    card_id.hpp
    card_definition.hpp
    card_registry.hpp
    pokemon/
      <one-card>.hpp
    trainers/
      <one-card>.hpp
    energy/
      <one-card>.hpp
  rules/
    card_context.hpp
  trace_engine_v2/
    core/
      card_context_adapter.inc
```

### `card_id.hpp`

Own the existing `sim::Card` enum in one reusable header. During bootstrap, move the enum without renaming enumerators or changing their meaning. Existing engine code continues to use `Card::QuickBall`, `Card::Crispin`, and the other current identifiers.

Do not introduce a second ID system during cleanup. A future external/public identifier is represented by the canonical print ID stored in `CardDefinition`.

### `card_definition.hpp`

This is the centralized metadata schema. It stores facts about the exact modeled print that are intrinsic to the card and are currently duplicated across catalog/classification switches.

The bootstrap schema should have the following shape:

```cpp
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "card_id.hpp"

namespace sim::cards {

enum class CardKind : std::uint8_t {
  Pokemon,
  Trainer,
  Energy,
};

enum class TrainerKind : std::uint8_t {
  None,
  Item,
  Supporter,
  Stadium,
  Tool,
};

enum class PokemonStage : std::uint8_t {
  None,
  Basic,
  Stage1,
  Stage2,
  VStar,
};

enum class PokemonType : std::uint8_t {
  None,
  Grass,
  Fire,
  Water,
  Lightning,
  Psychic,
  Fighting,
  Darkness,
  Metal,
  Dragon,
  Colorless,
};

struct CardDefinition {
  Card id;
  std::string_view canonical_id;
  std::string_view name;
  CardKind kind;
  TrainerKind trainer_kind{TrainerKind::None};
  PokemonStage pokemon_stage{PokemonStage::None};
  std::array<PokemonType, 2> pokemon_types{};
  std::uint8_t pokemon_type_count{};
  std::uint8_t retreat_cost{};
  bool rule_box{};
  bool pokemon_v{};
  bool ace_spec{};
  bool basic_energy{};
  std::string_view source_url;
};

}  // namespace sim::cards
```

Keep this schema about printed/canonical card facts. Do not put deck-specific strategy roles in it. In particular, concepts such as `is_payload`, strict-JIT value, DCI priority, matchup-flex value, route preference, setup-axis value, or Supporter contention belong to simulator policy, not to a reusable card definition.

If a later card needs a new intrinsic metadata field, add the smallest generic field that describes a real card property. Do not add a field whose name contains a specific card name or a specific Regidrago route.

### `card_registry.hpp`

The registry is explicit and deterministic. It contains the migrated card definitions and provides a lookup such as:

```cpp
const CardDefinition* find_definition(Card card);
```

During the header-only incremental phase, the registry may be `constexpr`/`inline` and include each migrated card header explicitly.

Do **not** use static-constructor self-registration. Do **not** rely on global initialization order. Do **not** require linker retention tricks. Each migration adds exactly one explicit registry entry.

Legacy helpers remain valid while migration is incomplete. Helpers such as `name()`, `is_basic()`, `is_pokemon()`, `is_pokemon_v()`, `is_rule_box_pokemon()`, `is_dragon()`, `is_basic_energy()`, `is_energy()`, `is_supporter()`, `is_item()`, `is_tool()`, `is_stadium()`, `is_ace_spec()`, and `retreat_cost()` must consult the registry first and fall back to the existing legacy switch for cards that have not migrated yet.

That compatibility bridge is what makes one-card cleanup migrations safe.

### `card_context.hpp`

Card modules must not directly own or reach into the Regidrago simulator's raw `State`, hand/deck/discard vectors, scenario policy, or trace-engine private members.

`CardContext` is a narrow rules-facing interface used by card effects. The bootstrap should contain only the generic operations needed by the Quick Ball reference migration. Add new operations later only when another card requires a genuinely reusable rules primitive.

Examples of acceptable context operations are:

```cpp
virtual int hand_count(Card card) const = 0;
virtual bool discard_from_hand(Card card, std::string_view reason) = 0;
virtual bool search_deck_to_hand(Card card) = 0;
virtual void shuffle_deck() = 0;
```

Future cards may justify generic operations such as deck-to-discard search, discard-to-hand recovery, bench placement, attachment, evolution, switching, Prize inspection/recovery, or an in-play target reference.

Do not add `resolve_quick_ball()`, `can_use_crispin()`, or any other card-specific context method. The context exposes rules primitives. The individual card module composes those primitives into its printed effect.

### `card_context_adapter.inc`

The existing `Engine` remains the owner of simulation state and policy during the migration. `card_context_adapter.inc` adapts the current Engine helpers/state transitions to the generic `CardContext` interface.

This adapter is the only compatibility layer that may know both the reusable card/rules API and trace-engine internals. Code under `src/cards/` must not include `trace_engine_v2` implementation files.

Preserve existing trace ordering and state mutation semantics through the adapter whenever possible. The migration itself must not silently change simulation behavior.

## Card module contract

Each migrated card gets exactly one primary module under `src/cards/pokemon/`, `src/cards/trainers/`, or `src/cards/energy/`.

A card module owns:

- the exact modeled print's `CardDefinition`;
- card-specific action/choice data needed to resolve that printed effect;
- validation of intrinsic card costs, target categories, choice counts, and printed play restrictions;
- resolution of the printed effect using generic `CardContext` operations;
- direct source identity for the modeled print.

A card module does not own:

- when the Regidrago planner prefers to play the card;
- which legal target is strategically best;
- strict-JIT or matchup-flex timing policy;
- DCI discard ranking;
- Supporter contention decisions;
- issue/seed-specific route selection;
- scheduled scenario lock policy;
- setup-readiness or payload policy.

The rule for every migration is:

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

If legacy code mixes those responsibilities in one block, leave the strategy/selection logic in the Engine and replace only the printed legality/effect portion with a call into the card module.

## Reference migration: Quick Ball

Quick Ball is the bootstrap reference because it demonstrates metadata, a mandatory hand cost, an optional deck-search result, a search filter, and shuffle behavior while still leaving DCI/target preference in the Engine.

Canonical print:

```text
Quick Ball — swsh1-179
https://api.pokemontcg.io/v2/cards/swsh1-179
```

The individual card module should have this conceptual public shape:

```cpp
#pragma once

#include <optional>

#include "../card_definition.hpp"
#include "../../rules/card_context.hpp"

namespace sim::cards {

class QuickBall final {
 public:
  struct Action {
    Card discard;
    std::optional<Card> search_target;
  };

  static constexpr CardDefinition definition{
      .id = Card::QuickBall,
      .canonical_id = "swsh1-179",
      .name = "Quick Ball",
      .kind = CardKind::Trainer,
      .trainer_kind = TrainerKind::Item,
      .source_url = "https://api.pokemontcg.io/v2/cards/swsh1-179",
  };

  static bool validate(const CardContext& context, const Action& action);
  static bool resolve(CardContext& context, const Action& action);
};

}  // namespace sim::cards
```

The Engine still decides *which* card to discard and *which* Basic Pokémon to search for. That includes all DCI, strict-JIT, redundancy, connector, lock-window, and route-priority reasoning.

`QuickBall::validate()` checks only the intrinsic request:

- the selected discard cost is actually payable from hand as an **other** card;
- if a search target is supplied, it is a Basic Pokémon according to the registry/legacy compatibility classifiers;
- the action shape is legal for Quick Ball.

`QuickBall::resolve()` performs only the printed card-specific work through `CardContext`:

1. pay the one-card hand discard cost;
2. search for the selected Basic Pokémon when one was selected and present;
3. allow a legal failed/empty search rather than inventing a target;
4. shuffle after the search effect.

Generic Item play restrictions, source-card lifecycle, global Item locks, and trace-engine bookkeeping should be handled by the rules/Engine compatibility layer in the same place they are handled for other Items. Do not duplicate global Item rules inside every Item class.

The bootstrap implementation must add focused Quick Ball tests before this cleanup playbook becomes active. Later agents should copy the **separation of responsibilities** demonstrated by Quick Ball, not copy Quick Ball-specific action fields into unrelated cards.

## Selecting one cleanup card

Choose exactly one card per cleanup issue and PR.

A card is eligible when all of the following are true:

1. `Card::<Name>` already exists in the simulator and the legacy engine already models the card in some form.
2. The card does not yet have a migrated module registered in `src/cards/card_registry.hpp`.
3. Its exact modeled print and rules source can be identified from repository source comments, `docs/CARD_AUDIT.md`, `docs/RULE_SOURCES.md`, `docs/RULES_TRACEABILITY.md`, or another authoritative source already used by the repository.
4. There is no active card-class migration issue already claiming that card.
5. The migration can be behavior-preserving. If you discover a real gameplay/card-text bug while preparing the migration, keep the cleanup migration separate and follow the normal bug workflow for the behavioral correction.

Prefer a small card whose printed effect can be separated cleanly from existing route policy. Search Items, simple Supporters, metadata-only Energy, and simple Pokémon Abilities are good early candidates. Leave highly entangled multi-mode or opponent-dependent cards until the rules/context primitives they require already exist.

## File and claim the enhancement issue first

Search open issues before creating a new one. If the same card already has an active card-class migration issue, do not duplicate or take over that work unless normal repository claim rules explicitly allow it.

Use this title:

```text
Enhancement: migrate <Card Name> to card class architecture
```

Use this issue body structure:

```markdown
## Card

- Card: <Card Name>
- Canonical print: `<set-id>`
- Source: <authoritative card URL>

## Current legacy ownership

- Metadata/classification: <legacy file/functions>
- Printed effect/resolution: <legacy file/functions>
- Strategy/policy that must remain in Engine: <legacy file/functions or brief description>

## Migration scope

Move this card's intrinsic metadata and printed effect into its individual `src/cards/...` module, register its `CardDefinition`, route legacy classification through the registry compatibility bridge, and make the existing Engine call the new card implementation for resolution.

Do not change route choice, DCI/JIT policy, scenario behavior, deck recipes, or unrelated cards.

## Acceptance criteria

- [ ] Exact metadata is represented by `CardDefinition`.
- [ ] Card is explicitly registered once.
- [ ] Legacy metadata/classification cases for this card are removed where the registry now owns them.
- [ ] Printed effect logic is owned by the card module.
- [ ] Strategy/target-selection policy remains in the Engine/policy layer.
- [ ] Focused legality/effect tests pass.
- [ ] Existing simulator regression tests pass with no unexplained output drift.
- [ ] Relevant `--simulate-this` traces preserve the same legal decisions/readiness behavior.
```

After filing, claim the issue using the repository's normal claim protocol before editing. The cleanup instruction authorizes creation of this migration enhancement; it does not authorize bypassing another agent's active claim.

## Implementation procedure

### 1. Map the card before moving anything

Search every occurrence of `Card::<Name>` and classify each occurrence as one of:

- **intrinsic metadata**;
- **printed legality/effect**;
- **generic rules/state transition**;
- **strategy/policy**;
- **test/reporting/documentation**.

Put this map in the issue or working notes. Do not assume that every occurrence belongs in the card module.

### 2. Add the individual card module

Create one file in the appropriate card directory. Define its `CardDefinition` and the smallest card-specific `Action`/choice structure needed for its effect.

Do not copy simulator policy helpers into the card file.

### 3. Register it explicitly

Add one include/entry to `card_registry.hpp`. Preserve deterministic explicit registration.

Add a focused registry test that proves the card is found by `Card` and that its canonical ID/name/classification fields match the exact modeled print.

### 4. Remove migrated metadata from legacy switches

Once registry lookup owns a metadata/classification fact for the card, remove that card's duplicate legacy case for the same fact.

Do not delete the legacy fallback switch itself. Other cards still depend on it until they migrate.

For derived classifiers, prefer deriving from `CardDefinition` rather than copying booleans. Examples:

- `is_basic()` derives from Pokémon stage;
- `is_item()`/`is_supporter()`/`is_stadium()`/`is_tool()` derive from trainer kind;
- `is_dragon()` derives from Pokémon type;
- `is_pokemon_v()` and `is_rule_box_pokemon()` derive from intrinsic flags;
- `retreat_cost()` derives from metadata;
- `is_basic_energy()` derives from energy metadata.

Keep model-specific roles such as `is_payload()` outside this schema.

### 5. Extract printed resolution, leave strategy in place

At the legacy execution point, keep the existing route selector and target/discard choice logic. Convert its final legal action into the card module's `Action` type and call the card implementation.

A cleanup migration must not make the card module decide which target the Regidrago strategy prefers.

If the card needs a state transition that `CardContext` cannot express, add the smallest reusable rules primitive to `CardContext` and implement it in `card_context_adapter.inc`. The primitive must describe a general game operation, not the migrating card.

### 6. Preserve trace and knowledge boundaries

Do not let reusable card code inspect hidden deck/Prize contents for strategy decisions. K0/K1 and other knowledge-policy decisions remain in the simulator layer.

During legal resolution, a context operation may inspect the zone necessary to carry out the effect. Keep that access behind the rules/context API so card code does not gain a direct pointer/reference to raw simulator containers.

Preserve existing trace events and ordering unless a separate approved issue explicitly changes them.

### 7. Test the card at three levels

**Focused card tests** must cover metadata plus the main legality/effect boundaries of the migrated card. Include at least one legal case and one rejected/edge case.

**Compatibility/regression tests** must prove existing callers still get the same `name()`/classification/retreat results and that the full test suite remains green apart from failures already documented on `main` before the migration.

**Simulation traces** must run relevant `--simulate-this` scenarios/seeds that exercise the card. Prefer at least three existing known-good traces when the card participates in registered recipes. Compare the important decisions, state transitions, and ready turn before/after. If the card cannot be exercised by three registered-recipe traces, document why and compensate with focused effect tests rather than fabricating irrelevant traces.

For a pure migration, unexplained behavior, matrix, or trace drift is a failure. Stop and identify whether the migration accidentally moved policy or uncovered a separate gameplay bug.

### 8. Keep the PR atomic

One cleanup PR migrates one card. It may contain only:

- that card's module;
- its single registry entry;
- removal of that card's duplicate legacy metadata/effect code;
- a minimal generic `CardContext` extension required by that card;
- focused tests and directly related docs.

Do not opportunistically migrate neighboring cards, reorganize the full composition pipeline, rename broad APIs, or introduce new library/CMake targets in the same PR.

## Completion checklist

A card is migrated only when all of these are true:

- [ ] one enhancement issue exists and is claimed;
- [ ] one individual card module owns the exact print metadata;
- [ ] the registry has exactly one explicit entry for the card;
- [ ] legacy metadata/classification duplicates for the migrated facts are removed;
- [ ] the printed effect/resolution is invoked through the card module;
- [ ] reusable card code does not access raw Engine state containers;
- [ ] strategy policy remains outside the card module;
- [ ] authoritative card/rule sources remain traceable;
- [ ] focused tests pass;
- [ ] full regression tests pass, subject only to pre-existing documented `main` failures;
- [ ] relevant simulation traces show no unexplained behavioral drift;
- [ ] no unrelated card was migrated in the same PR.

## What happens after enough cards migrate

Do not perform this step during ordinary card cleanup.

Once a substantial set of cards is migrated and the compatibility bridge is proven, a separate architecture change may turn the header-shaped `src/cards` and `src/rules` code into real CMake library targets, move implementations from headers to `.cpp` files where useful, and retire the remaining `.inc` card-model switches after the last legacy card migrates.

That later change should be mechanical because cleanup migrations have already established the dependency direction:

```text
rules <- cards <- simulator/strategy
```

The incremental cleanup phase exists to reach that point without a one-shot rewrite.
