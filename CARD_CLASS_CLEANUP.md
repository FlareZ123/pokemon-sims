# Card Class Cleanup

This document describes the card-class architecture that exists in the repository today and the cleanup constraints for extending it.

The simulator currently has a reusable card metadata layer, a narrow rules context, and a card module for Quick Ball. Strategy and scenario policy live in the trace engine. New card-class coverage should keep those ownership boundaries intact.

## Current architecture

```text
src/
  cards/
    card_id.hpp
    card_definition.hpp
    card_registry.hpp
    trainers/
      quick_ball.hpp
  rules/
    card_context.hpp
  trace_engine_v2/
    core/
      card_context_adapter.inc
      quick_ball_card_class_base.inc
      quick_ball_card_class_tail.inc
```

The dependency direction is:

```text
rules <- cards <- simulator policy
```

Strategy selects an action. Card code validates and resolves the printed effect. Reusable rules operations perform state transitions.

## Card identity

`src/cards/card_id.hpp` owns `sim::Card`, the simulator's stable card identifier. Exact-print identity belongs in `CardDefinition::canonical_id`.

## Intrinsic metadata

`src/cards/card_definition.hpp` stores facts intrinsic to the modeled print, including card kind, Trainer subtype, Pokémon stage and type, retreat cost, Rule Box status, Pokémon V status, ACE SPEC status, Basic Energy status, and the direct source URL.

Strategy properties stay outside `CardDefinition`. Payload value, DCI priority, JIT timing, connector priority, matchup value, route preference, and readiness policy depend on simulator state.

When a card needs metadata that the schema does not contain, add the smallest reusable field that represents an intrinsic card fact.

## Registry

`src/cards/card_registry.hpp` owns explicit registration. The current registry contains Quick Ball.

Registration stays deterministic and visible in source. Add explicit entries for newly covered cards. Code that can derive a fact from `CardDefinition` should use the registered definition instead of duplicating that fact elsewhere.

## Rules context

`src/rules/card_context.hpp` is the card layer's access point to game state. It is a concrete callback-backed object. Card modules receive this narrow interface instead of raw `Engine`, `State`, or zone containers.

The current context supports hand counts, hand-to-discard movement, discard-cost payment with trace metadata, deck-to-hand search, deck shuffling, Basic Pokémon classification, and deck-search knowledge transitions.

Extend `CardContext` only when another printed effect needs a reusable game operation. Generic operations such as deck-to-discard search, Bench placement, attachment, evolution, switching, recovery, or Prize inspection belong here when they become necessary. Card-specific strategy does not.

## Adapter

`src/trace_engine_v2/core/card_context_adapter.inc` connects the trace engine to `CardContext`. It owns integration details such as trace ordering, knowledge transitions, zone mutation, and shuffle behavior where those details are part of the engine contract.

## Card modules

A card module owns the printed behavior for one exact modeled card. That includes `CardDefinition` metadata, intrinsic play legality, printed costs, target categories, choice cardinality, effect resolution through `CardContext`, and direct source identity.

Regidrago strategy remains in the trace engine. Target preference, discard ranking, setup-axis priority, lock-aware route selection, knowledge-aware planning, JIT policy, Supporter contention, and readiness policy stay outside the card module.

## Quick Ball reference

Quick Ball `swsh1-179` is the complete card-class example currently in the repository:

https://api.pokemontcg.io/v2/cards/swsh1-179

Its card module represents the printed Item transaction. Engine policy supplies the discard choice and Basic Pokémon search preference. The card layer validates the transaction and resolves it through `CardContext`.

Primary files:

```text
src/cards/trainers/quick_ball.hpp
src/cards/card_registry.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/card_context_adapter.inc
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

## Adding card-class coverage

When another modeled card is placed behind the card-class interface:

1. Record the exact print and authoritative source.
2. Add intrinsic metadata in the appropriate card module.
3. Resolve the printed effect through reusable `CardContext` operations.
4. Keep strategy and route selection in `trace_engine_v2`.
5. Register the card explicitly.
6. Remove duplicate intrinsic classification data once the registry owns the same fact.
7. Preserve legal outcomes, knowledge transitions, trace semantics, and route-selection behavior unless the change intentionally fixes gameplay.
8. Add direct card tests and engine integration tests for the new boundary.

## Design constraints

- Keep one stable `Card` enum.
- Keep one explicit card registry.
- Put intrinsic metadata in `CardDefinition`.
- Keep strategy outside card metadata.
- Put reusable game operations in `CardContext`.
- Keep raw `Engine` and `State` access out of `src/cards/`.
- Avoid card-specific context methods that bypass the ownership boundary.
- Avoid hidden global registration side effects.
- Remove duplicate sources of truth for registered intrinsic card facts.
- Keep route-specific behavior out of reusable card resolvers.

## Source requirements

Rule-sensitive production code should keep a direct authoritative source URL beside the relevant implementation. Use the exact modeled print when card text matters. Repository policy sources are appropriate for simulator-specific decisions such as DCI, JIT, knowledge states, locks, and readiness.

Primary references:

- [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md)
- [`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md)
- [`docs/MODEL_ASSUMPTIONS.md`](docs/MODEL_ASSUMPTIONS.md)
- [`docs/POLICY_DECISIONS.md`](docs/POLICY_DECISIONS.md)
- [`EN_advanced_manual-2025-transcription-structured.md`](EN_advanced_manual-2025-transcription-structured.md)

## Validation

Card-class cleanup should use the normal repository validation surface: C++20 compilation, focused card and integration tests, the full CTest suite, sanitizer coverage for state-transition changes, deterministic trace inspection for affected routes, and regenerated source-bound statistical evidence when behavior or policy inputs change.

The architecture should read as one current system whose card metadata, printed effects, rules operations, and setup strategy have clear owners.
