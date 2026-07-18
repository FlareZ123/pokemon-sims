# pokemon-sims

A traceable, rules-mapped goldfish simulator for the supplied Pokémon TCG Live Expanded Regidrago VSTAR deck.

## Audit status

This repository contains a rules-mapped simulator, deterministic `simulate-this` traces, six scenario trace regressions, **57 core exact-state policy fixtures**, and a **31-case Tier Two** suite for choice differentiation and fast-compression states. The executable fixture tables are the canonical inventories: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191 and https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73. The synchronized indexes are in [`docs/OPTIMAL_POLICY_FIXTURES.md`](docs/OPTIMAL_POLICY_FIXTURES.md) and [`docs/TIER2_POLICY_FIXTURES.md`](docs/TIER2_POLICY_FIXTURES.md). The K0/K1 knowledge model and connector hierarchy are recorded in [`docs/POLICY_DECISIONS.md`](docs/POLICY_DECISIONS.md). Exact direct card-text and procedure URLs are in [`docs/RULE_SOURCES.md`](docs/RULE_SOURCES.md), with the same raw URLs placed in the active source wrapper. The card-text recheck is in [`docs/RULES_REVERIFICATION.md`](docs/RULES_REVERIFICATION.md).

The model is deliberately a single-player setup model. It does not claim to resolve every Expanded card or any opponent-dependent game state.

## Run one readable hand

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 1 --require-ready-by 3
```

The command prints the opening player-known state, debug-only prizes, every draw, card cost, search, attachment, evolution, retreat, VSTAR Power, attack, and ready-state check. The debug Prize list is never read by policy before a legal reveal/search effect.

## Run exact-state policy tests

```bash
ctest --test-dir build --output-on-failure
./build/regidrago_unified_tests regidrago_policy_tests
./build/regidrago_unified_tests regidrago_tier2_tests
```

The `regidrago_policy_tests` unified case checks core rule and connector legality. The `regidrago_tier2_tests` unified case checks comparison states, including direct Crispin versus indirect Energy routes, Steven versus Arven, K0/K1 Gladion and Heavy Ball choices, Heavy Ball into Tapu Lele-GX Supporter chains, Forest Seal Stone conservation, Rule Box lock-aware search paths, Tate & Liza switch and draw modes, and Arven fallback selection.

## Run aggregate smoke test

```bash
./build/regidrago_sim --trials 100000 --seed 20260705 --out results/simulation_results.csv
```

## T5 horizon and setup-loss rule

Standard scenarios stop early when they become ready through T4. Games still unresolved after T4 continue through T5 for diagnostic recovery data. Reaching T5 is classified as a setup loss even when the deck first becomes ready during that turn.

Aggregate CSV output separates cumulative `ready_by_t5_pct`, T5-only `ready_on_t5_pct`, and the benchmark `setup_failure_pct`. The failure percentage is the complement of readiness through T4. A T5 recovery remains inside that failure count.

## Scope

A ready state means:

1. Active Regidrago VSTAR.
2. At least two Grass and one Fire attached.
3. A modeled Dragon payload in discard: Dragapult ex, Mega Dragonite ex, Dialga-GX, or Hisuian Goodra VSTAR.
4. In strict and matchup-flex profiles, that payload entered discard in the current player turn and remains in discard at the ready check.

See `docs/T5_FAILURE_POLICY.md`, `docs/RULES_TRACEABILITY.md`, `docs/RULES_REVERIFICATION.md`, `docs/RULE_SOURCES.md`, `docs/POLICY_DECISIONS.md`, `docs/OPTIMAL_POLICY_FIXTURES.md`, `docs/TIER2_POLICY_FIXTURES.md`, `docs/TRACE_AUDIT.md`, `docs/MODEL_ASSUMPTIONS.md`, and `docs/REPORT.md`.
