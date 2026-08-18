# Card Class Cleanup

This is the live architecture and migration plan. Historical cleanup details remain in Git history.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Bootstrap gate

Quick Ball remains the migration reference for registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, resolving-source movement, failed-search behavior, shuffle, and trace compatibility.

Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179
Advanced Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Current ownership

- `src/cards/card_id.hpp`, `card_definition.hpp`, and `card_registry.hpp` own stable IDs, intrinsic print facts, and deterministic registration.
- `src/rules/card_context.hpp` owns reusable printed-rules operations and generic played-Trainer source lifecycle.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` bridges reusable card effects into the trace engine.
- Engine strategy retains route admission, target preference, DCI/UDP/AMR, strict-JIT timing, Supporter contention, connector domination, K0/K1 state, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner.
- `src/trace_engine_v2/core/forretress/scenario_registry.hpp` owns the stable Forretress scenario registry aliases and declarations.
- `src/trace_engine_v2/core/forretress/package.inc` owns Forretress runtime composition plus the Garbodor / Boost Shake scenario extension family.

Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` remains the compatibility catalog for unmigrated metadata fallbacks. `src/trace_engine_v2/core/deck_knowledge.inc` owns copy arithmetic after visibility is legally resolved. Hidden-zone visibility and K0/K1 route admission remain Engine concerns.

Canonical catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
Canonical knowledge owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/deck_knowledge.inc

## Shared policy owners

Pure projections belong in named final policy owners rather than repeated Engine scans. Current shared seams include `core/locks/garbodor_policy.inc`, `core/board_state_policy.inc`, `core/scenario_extension_policy.inc`, and named route policies under `core/routes/`.

Repository policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` owns shared Dragon-payload zone membership and preference traversal. Engine retains DCI/JIT timing, K0/K1 knowledge, connector domination, discard legality, and physical card movement. Strict-JIT routes must count a current-turn payload only after it is legally in the discard pile.

Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
Canonical payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup/policies.inc` owns pure setup recipe classification and setup constants. `src/trace_engine_v2/core/setup_lifecycle.inc` owns opening hand, mulligan, Prize deal, and setup trace state changes. Preserve opening legality, Prize isolation, seeded reproducibility, and the first legal deck-search transition from K0 to K1.

Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620

Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine during card migrations.

## Composition ownership

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots.

C++ textual include semantics: https://eel.is/c++draft/cpp.include

Prefer normal `.hpp` owners for stable declarations and pure policy classes. Keep `.inc` only where textual inclusion is still required by the monolithic Engine class or an intentional macro-composition boundary.

The Forretress cleanup now separates its stable scenario-registry contract from textual composition. `scenario_registry.hpp` owns aliases and declarations. `package.inc` retains runtime and concrete scenario-extension composition. Scenario order, labels, seeded comparisons, and rule-sensitive runtime logic stay unchanged by this boundary move.

## Next cleanup steps

1. Migrate the remaining `part_014c.inc` Forretress consumer to `core/forretress/package.inc` directly, then delete `part_forretress_ex_combo.inc` after full matrix validation.
2. Split stable declarations and pure projections out of `core/forretress/runtime.inc` into normal headers or final policy classes.
3. Reduce `composition/post_014a_overrides.inc` by moving coherent route-policy groups into named owners under `core/routes/` while preserving composition order.
4. Retire old Steven and Tate compatibility mirrors after all source-contract references point at their canonical owners.
5. Continue card migrations one card at a time with exact-print, legality, resolution, trace, and strategy-separation tests.

## Validation gate

Every behavior-preserving cleanup must pass the strict C++20 warning gate, all deterministic `--simulate-this` audits, exact 100,000-trial paired-matrix comparison, the complete Release suite, and sanitizer CI. Preserve K0/K1 visibility, DCI/JIT timing, Supporter contention, connector domination, zone integrity, scenario ordering, and seeded reproducibility.

Rules source: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
Repository policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md
