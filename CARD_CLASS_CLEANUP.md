# Card Class Cleanup

**Cleanup directive:** read this file, file and claim an enhancement issue for exactly one card that is already modeled by the simulator but has not yet been migrated into the card-class architecture, then migrate only that card using the architecture and compatibility rules below.

In this document, **unimplemented card** means **not yet implemented in the new card-class architecture**. The card may already be modeled by the legacy simulator. This workflow is a behavior-preserving migration task. It does not authorize unrelated gameplay changes or strategy-policy changes.

## Bootstrap gate

The architecture skeleton and complete Quick Ball reference path must exist before an agent starts another card migration.

Verify these files:

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

If the skeleton or active Quick Ball reference path is missing, stop and report that the bootstrap is incomplete. Do not invent another class hierarchy, registration mechanism, side-effectful registry, or CMake library layout.

## Composition consolidation status

Late-search composition now has one explicit owner: `src/trace_engine_v2/composition/post_014a_overrides.inc`.

The former one-purpose wrappers `composition/empty_deck_search_routes.inc` and `composition/post_search_connector_routes.inc` have been merged into that owner without changing their textual include order, macro guards, alias lifetimes, or gameplay entry points. This reduces wrapper depth while keeping the behavior-sensitive legacy sequence auditable at one class-member boundary.

The former `composition/legacy_supporter_body.inc` wrapper is now merged into its sole owner, `composition/opening_engine_overrides.inc`. The `part_011` / `part_012` / `part_013` order and all entry/exit guards remain at that boundary, while `use_fss` and `use_celestial_roar` continue across the boundary for `post_014a_overrides.inc` to consume. This removes another forwarding layer without changing the legacy declaration order.

Keep these cleanup rules going forward:

- when a composition-only `.inc` file has exactly one owner and exists only to forward an ordered macro/include block, prefer merging that block into the owner once the dependency boundary is proven by CI;
- preserve the original `#define` / `#include` / `#undef` order exactly during a mechanical merge;
- keep entry and exit macro guards beside the inlined block so alias ownership remains explicit;
- do not merge a transitional card bridge merely to reduce file count when its split marks a real declaration-order dependency;
- specifically, keep `quick_ball_card_class_base.inc` and `quick_ball_card_class_tail.inc` separate until the helpers consumed by the tail can be made available at a single safe member boundary without reordering legacy declarations;
- after each composition merge, require strict compilation, the full regression suite, and representative `--simulate-this` traces before merging.

C++ textual-include semantics used by these mechanical consolidations: https://eel.is/c++draft/cpp.include

## Why migration is incremental

The simulator is currently composed as one translation unit through `src/regidrago_sim.cpp` and ordered `.inc` files under `src/trace_engine_v2/`. Legacy metadata, classifiers, policy helpers, state transitions, and card effects still cross textual include boundaries.

Migrated and legacy cards therefore coexist. A one-card cleanup must not replace the composition pipeline, create a new linked library target, rename every `Card` use, or migrate neighboring cards.

The reusable source layout is intentionally header-shaped so a later architecture change can turn it into normal library targets after enough cards migrate.

## Dependency direction

The migration must preserve this direction:

```text
rules <- cards <- simulator/strategy
```

The operating rule is:

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

A card module must never reach back into trace-engine policy or raw simulator state.

## Target layout

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
      <transitional card bridge>.inc
```

## Core architecture contracts

### `card_id.hpp`

`src/cards/card_id.hpp` owns the stable existing `sim::Card` enum. Do not introduce a second identifier system during cleanup and do not renumber or rename existing enumerators.

The external exact-print identity belongs in `CardDefinition::canonical_id`.

### `card_definition.hpp`

`CardDefinition` stores intrinsic facts about the exact modeled print. Appropriate fields include:

- stable `Card` id;
- canonical print id;
- display name;
- card kind;
- Trainer subtype;
- Pokémon stage and type;
- retreat cost;
- rule-box / Pokémon V / ACE SPEC / Basic Energy flags;
- direct source URL.

Do not put model policy into `CardDefinition`. `is_payload`, DCI rank, strict-JIT value, connector priority, matchup-flex value, route preference, Supporter contention, and setup-axis value remain simulator strategy.

When another card needs new metadata, add the smallest generic intrinsic field that describes a real card property. Do not add a field named after a particular Regidrago route or a particular card effect.

### `card_registry.hpp`

Registration is explicit and deterministic. Every migrated card adds one explicit include/entry. Do not use static-constructor self-registration, global initialization side effects, or linker-retention tricks.

The compatibility layer must consult registered metadata before using legacy fallback tables. Once the registry owns a migrated fact, remove that card's duplicate legacy data for that fact.

For derived facts, prefer deriving from `CardDefinition`:

- Trainer subtype -> Item / Supporter / Stadium / Tool;
- Pokémon stage -> Basic;
- Pokémon type -> Dragon or another printed type;
- intrinsic flags -> Rule Box / Pokémon V / ACE SPEC;
- retreat metadata -> retreat cost;
- Energy metadata -> Basic Energy.

Model-specific roles such as `is_payload()` remain outside the registry schema.

### `card_context.hpp`

`CardContext` is the narrow reusable rules seam used by card modules. The current implementation is a concrete callback-backed object rather than a virtual base class. That keeps the transitional single-TU integration allocation-free and prevents card code from receiving raw `Engine` or `State` pointers.

The Quick Ball reference currently needs generic operations equivalent to:

```cpp
int hand_count(Card card) const;
bool move_hand_to_discard(Card card);
bool discard_from_hand(Card card,
                       std::string_view reason,
                       std::string_view rules_reference);
bool search_deck_to_hand(Card card);
void shuffle_deck();
bool is_basic_pokemon(Card card) const;
void begin_deck_search(std::string_view reason);
```

Future migrations may justify generic operations for deck-to-discard search, discard recovery, bench placement, attachment, evolution, switching, Prize inspection/recovery, or an in-play target reference.

Never add a card-specific context operation such as `resolve_quick_ball()` or `can_use_crispin()`. The context exposes reusable game/rules operations. The card module composes them.

### `card_context_adapter.inc`

The trace engine still owns simulator state and policy. `card_context_adapter.inc` bridges Engine callbacks into `CardContext`.

This compatibility layer may know both APIs. Code under `src/cards/` must not include `trace_engine_v2` implementation files or directly inspect `Engine::state_`.

Keep knowledge transitions, state mutations, shuffle behavior, and trace ordering compatible with the existing simulator unless a separate behavioral issue explicitly changes them.

## Card module contract

Each migrated card gets exactly one primary module under `src/cards/pokemon/`, `src/cards/trainers/`, or `src/cards/energy/`.

A card module owns:

- exact-print `CardDefinition` metadata;
- card-specific action/choice data required to resolve the printed effect;
- intrinsic cost legality;
- printed target categories and choice cardinality;
- printed effect resolution through `CardContext`;
- direct source identity for the modeled print.

A card module does not own:

- when the Regidrago planner wants to play the card;
- which legal target is strategically preferred;
- DCI discard ranking;
- strict-JIT or matchup-flex timing;
- Supporter contention;
- connector or setup-axis priorities;
- issue/seed route selection;
- scenario lock schedules;
- readiness or payload policy.

If a legacy function mixes these responsibilities, leave route admission, cost selection, and target preference in Engine policy. Move printed validation and resolution behind the card API.

## Reference migration: Quick Ball

Exact modeled print:

```text
Quick Ball — swsh1-179
https://api.pokemontcg.io/v2/cards/swsh1-179
```

Quick Ball is the reference because it exercises all of the important seams at once:

- exact-print metadata and registry lookup;
- Trainer/Item classification;
- a mandatory **other card** hand cost;
- a legal deck inspection that changes knowledge from K0 to K1;
- strategy-owned target selection after inspection;
- intrinsic Basic-Pokémon target filtering;
- a legal failed/empty search;
- shuffle after the search;
- source Item movement through a generic state primitive.

The current conceptual public shape is:

```cpp
class QuickBall final {
 public:
  using SearchTargetSelector = std::optional<Card> (*)(void*);

  struct Action {
    Card discard;
    void* search_context = nullptr;
    SearchTargetSelector choose_search_target = nullptr;
    std::string_view cost_reason = "Quick Ball cost";
    std::string_view rules_reference = "R-QB-01";
    std::string_view search_reason = "Quick Ball";
  };

  struct Resolution {
    bool played = false;
    std::optional<Card> search_target;
    bool found_target = false;
  };

  static constexpr CardDefinition definition{/* exact swsh1-179 metadata */};

  static bool validate(const rules::CardContext& context,
                       const Action& action);
  static Resolution resolve(rules::CardContext& context,
                            const Action& action);
};
```

The target selector is deliberately a strategy callback. `QuickBall::resolve()` starts the legal deck search first, which establishes K1 through `CardContext::begin_deck_search()`. Only then does it invoke the Engine-owned selector. This ordering is mandatory for hidden-information policy.

Do not precompute a Quick Ball deck target while the simulator is still at K0 merely to simplify the card API.

`QuickBall::validate()` checks that a source Quick Ball exists and that the selected mandatory discard is payable as an **other card**. Two Quick Ball copies therefore allow one copy to pay the other's cost; one copy cannot discard itself.

`QuickBall::resolve()` owns the printed transaction through generic rules primitives:

1. move the played Quick Ball from hand to discard;
2. discard the strategy-selected other-card cost;
3. begin the legal deck search and establish K1;
4. ask Engine strategy for the post-inspection preferred target;
5. accept only a Basic Pokémon target;
6. move the target to hand when present, while permitting a legal failed search;
7. shuffle the deck;
8. return a small resolution summary so Engine can preserve its existing trace wording.

Global Item locks, route admission, DCI/JIT policy, cost preference, target preference, connector policy, and trace narration remain in Engine/strategy code.

The active Quick Ball paths in `quick_ball_card_class_base.inc` and `quick_ball_card_class_tail.inc` route the printed transaction through `cards::QuickBall` and `CardContext`. Historical resolver text may temporarily remain compiled under a dormant compatibility name where the current `.inc` composition still needs interleaved policy helper declarations. That dormant code must not be an active gameplay entry point, and new migrations must not copy this transitional composition debt. Helper extraction/removal belongs in a separate mechanical cleanup when safe.

`tests/quick_ball_card_class_tests.cpp` is the focused reference test. Later migrations should copy the separation of responsibilities and testing depth, not Quick Ball-specific action fields.

## Selecting one cleanup card

Choose exactly one card per cleanup issue and PR.

A card is eligible only when all of these are true:

1. `Card::<Name>` already exists and the legacy simulator already models it.
2. It has no registered migrated module yet.
3. Its exact modeled print and authoritative source can be identified from repository evidence.
4. No active card-class migration issue already claims it.
5. The work can remain behavior-preserving.

If you discover a real card-text/gameplay bug during migration, keep that behavioral correction in the normal bug workflow rather than silently folding it into the architecture cleanup.

Prefer early cards whose printed effects separate cleanly from strategy. Leave highly entangled multi-mode or opponent-dependent cards until the generic context primitives they need exist.

## File and claim the enhancement issue first

Search open issues before filing. Do not duplicate another active migration or bypass an existing claim.

Use this title:

```text
Enhancement: migrate <Card Name> to card class architecture
```

Record at least:

```markdown
## Card
- Card: <Card Name>
- Canonical print: `<set-id>`
- Source: <authoritative card URL>

## Current legacy ownership
- Metadata/classification: <files/functions>
- Printed effect/resolution: <files/functions>
- Strategy/policy that must remain in Engine: <files/functions or description>

## Migration scope
Move only this card's intrinsic metadata and printed effect into its
`src/cards/...` module, register it explicitly, route compatibility classifiers
through registered metadata, and make the active Engine path call the card module.

Do not change route choice, DCI/JIT policy, scenario behavior, deck recipes, or
unrelated cards.

## Acceptance criteria
- [ ] Exact metadata is represented by `CardDefinition`.
- [ ] Card is explicitly registered once.
- [ ] Duplicate legacy metadata/classification ownership is removed.
- [ ] Active printed effect resolution is invoked through the card module.
- [ ] Strategy/target-selection policy remains in Engine/policy.
- [ ] Focused legality/effect tests pass.
- [ ] Existing regression tests have no unexplained drift.
- [ ] Relevant `--simulate-this` traces preserve legal decisions/readiness.
```

Claim the issue using the repository's normal claim protocol before editing.

## Implementation procedure

### 1. Map every occurrence

Search every `Card::<Name>` occurrence and classify it as:

- intrinsic metadata;
- printed legality/effect;
- generic rules/state transition;
- strategy/policy;
- test/reporting/documentation.

Do not assume every occurrence belongs in the card module.

### 2. Add one card module

Create one file in the appropriate card directory. Add the exact `CardDefinition` and the smallest card-specific action/choice structures needed for printed resolution.

Do not copy policy helpers into the card file.

### 3. Register it once

Add the module to `card_registry.hpp` with deterministic explicit registration.

Add a focused test proving registry lookup and exact canonical metadata.

### 4. Remove duplicate legacy metadata ownership

When registry metadata owns a migrated fact, remove that card's duplicate legacy case for the same fact. Keep the fallback table itself because unmigrated cards still use it.

### 5. Route active printed resolution through the card module

Keep route admission and strategic choice in Engine. Feed the chosen legal action into the card module, and let `CardContext` perform generic state transitions.

If a required operation is missing, add the smallest reusable context primitive. Its name and contract must describe a general game operation rather than the migrating card.

### 6. Preserve knowledge boundaries

Reusable card code must not inspect hidden deck/Prize contents for strategy. K0/K1 and other knowledge-policy decisions remain in the simulator layer.

Some cards, including Quick Ball, require a strategic choice only after a legal inspection changes knowledge. Model that as staged resolution or a strategy callback invoked after the context establishes the new knowledge state. Do not move the choice earlier.

A context operation may inspect the zone required to carry out a legal effect. Keep that access behind the rules/context API.

### 7. Preserve trace behavior

Maintain existing trace events, reasons, and ordering unless a separate approved behavioral issue changes them. Return small resolution facts from card code when Engine needs them to produce the same narration.

### 8. Test at three levels

**Focused card tests** cover exact metadata plus main legality/effect boundaries. Include legal and rejected/edge cases. Search cards should cover failed-search/shuffle behavior and any important knowledge-ordering seam.

**Compatibility/regression tests** prove existing callers still receive the correct name/classification/retreat facts and that the full suite has no unexplained new failures.

**Simulation traces** run relevant existing `--simulate-this` scenarios/seeds. Prefer at least three known-good traces when the card participates in registered recipes. Compare important decisions, state transitions, and ready turn. If three relevant traces do not exist, explain why and compensate with focused effect tests.

For a pure migration, unexplained behavior, matrix, or trace drift is a failure.

### 9. Keep the PR atomic

One cleanup PR migrates one card. It may contain only:

- that card module;
- its registry entry;
- removal of its duplicate legacy metadata/effect ownership;
- a minimal generic `CardContext` extension required by that card;
- focused tests;
- directly related documentation.

Do not migrate neighboring cards, broadly rename APIs, reorganize the composition pipeline, or introduce new library targets in the same PR.

## Completion checklist

A cleanup card migration is complete only when all applicable items are true:

- [ ] one enhancement issue exists and is claimed;
- [ ] one individual module owns exact-print metadata;
- [ ] the registry has exactly one explicit entry;
- [ ] migrated metadata/classification duplicates are removed;
- [ ] the active printed effect is invoked through the card module;
- [ ] reusable card code cannot access raw Engine state containers;
- [ ] strategy policy remains outside the card module;
- [ ] authoritative card/rule sources remain traceable;
- [ ] focused tests pass;
- [ ] full regression tests pass subject only to documented pre-existing `main` failures;
- [ ] relevant simulation traces have no unexplained behavioral drift;
- [ ] no unrelated card was migrated in the same cleanup.

## Later library extraction

Do not perform library extraction during ordinary card cleanup.

After a substantial set of cards migrates and the compatibility bridge is proven, a separate architecture change may create normal CMake library targets, move implementations into `.cpp` files where useful, and retire the remaining legacy `.inc` card-model tables after the last card migrates.

The incremental phase exists so that later extraction is mechanical rather than a one-shot gameplay rewrite.