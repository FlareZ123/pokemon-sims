# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup details remain in Git history. Keep this file focused on current ownership, remaining boundaries, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Bootstrap gate

Keep the Quick Ball reference seam intact while other cards migrate:

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

Quick Ball remains the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, resolving-source movement, failed-search behavior, shuffle, and trace compatibility.

Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179
Advanced Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Current ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. External print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts and reusable intrinsic classification.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical lookup.
- `src/rules/card_context.hpp` owns reusable printed-rules operations and the generic played-Trainer resolving-source lifecycle.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` is the trace-engine bridge for reusable card effects.
- Engine strategy retains route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner.
- `src/trace_engine_v2/core/locks/garbodor_policy.inc` owns Garbodor scenario timing and Ability-lock policy. Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
- `src/trace_engine_v2/core/payload_hand_policy.inc` owns shared Dragon-payload zone and preference queries.
- `src/trace_engine_v2/core/board_state_policy.inc` owns reusable board traversal and board-index queries.
- `src/trace_engine_v2/core/setup/policies.inc` owns pure setup recipe classification and setup constants. `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening-hand, mulligan, Prize-deal, and setup-trace state changes.
- `src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn action-state reset semantics.
- `src/trace_engine_v2/core/deck_knowledge.inc` owns reusable copy arithmetic after visibility is resolved.
- `src/trace_engine_v2/core/routes/` owns named route policies. Physical card effects continue moving toward `src/cards/` plus `CardContext` primitives while strategic route policy remains in Engine.
- `src/trace_engine_v2/core/scenario_extension_policy.inc` owns generic scenario append, lookup, range ownership, and extension-first fallback mechanics.
- `src/trace_engine_v2/core/forretress/scenario_registry.hpp` now owns the namespace-scope Forretress scenario registry aliases and declarations. This moves stable declarations out of textual `.inc` composition without changing scenario order or behavior. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/scenario_registry.hpp
- `src/trace_engine_v2/core/forretress/package.inc` remains the sole Forretress namespace-scope composition owner for runtime plus the Garbodor / Boost Shake extension family. Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move intrinsic metadata and classification before printed resolution. Move printed resolution only after the live resolver and reusable `CardContext` operations are identified. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine.

Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Knowledge and route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

## Composition ownership

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots.

C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Prefer normal `.hpp` owners for stable declarations and pure policy classes. Keep `.inc` files only where textual inclusion is still required by the monolithic Engine class or by an intentional macro-composition boundary. New compatibility shims need a concrete removal step in this document.

The Forretress cleanup now separates its stable scenario-registry contract from its textual package. `scenario_registry.hpp` owns aliases and declarations, while `package.inc` owns the runtime and concrete scenario-extension composition. Scenario order, labels, seeded common-random-number behavior, and rule-sensitive runtime logic remain unchanged by this boundary move.

## Next cleanup steps

1. Migrate the remaining `part_014c.inc` Forretress consumer to `core/forretress/package.inc` directly, then delete `part_forretress_ex_combo.inc`. Preserve the exact namespace-scope include point and run the full matrix before removal.
2. Split stable declarations and pure projections out of `core/forretress/runtime.inc` into normal headers or final policy classes. Keep state mutation and printed Ability resolution together until their call contracts are explicit.
3. Reduce `composition/post_014a_overrides.inc` by moving coherent route-policy groups into named owners under `core/routes/`, preserving textual order at the composition boundary.
4. Retire `part_issue_1199_steven_package_override.inc` after all source-contract references point at `core/routes/issue_1199_steven_package_policy.inc`.
5. Retire the remaining Tate provenance and attachment source-contract mirrors after references point at `core/tate/` owners.
6. Continue card migrations one card at a time. A migration is complete only when exact-print metadata, printed legality, resolution, trace compatibility, and strategy separation are covered by tests.

## Validation gate

Every cleanup branch must preserve observable simulator behavior unless the branch is explicitly fixing a confirmed bug. Before merge:

- build with the repository's strict C++20 warning gate;
- run all deterministic `--simulate-this` policy audits and inspect their traces for legal, resource-aware play;
- run the paired 100,000-trial T2/T3 matrix and require exact equality with the committed baseline for behavior-preserving cleanup;
- run the complete Release test suite;
- run sanitizer CI;
- preserve K0/K1 visibility boundaries, DCI/JIT timing, Supporter contention, connector domination, card-zone integrity, scenario ordering, and seeded reproducibility.

Rules source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Repository policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
