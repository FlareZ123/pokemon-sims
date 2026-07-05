# pokemon-sims

A traceable, rules-mapped goldfish simulator for the supplied Pokémon TCG Live Expanded Regidrago VSTAR deck.

## Audit status

`fix-sims` contains a rules-mapped simulator, deterministic `simulate-this` traces, six scenario trace regressions, and **28 exact-state policy fixtures**. The fixture catalog states each constructed hand, board, deck, Prize, discard, lock, expected action, and expected resulting state in [`docs/OPTIMAL_POLICY_FIXTURES.md`](docs/OPTIMAL_POLICY_FIXTURES.md). The card-text recheck is in [`docs/RULES_REVERIFICATION.md`](docs/RULES_REVERIFICATION.md).

The model is deliberately a single-player setup model. It does not claim to resolve every Expanded card or any opponent-dependent game state.

## Run one readable hand

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 6 --require-ready-by 3
```

The command prints the opening player-known state, debug-only prizes, every draw, card cost, search, attachment, evolution, retreat, VSTAR Power, attack, and ready-state check. The debug Prize list is never read by policy before a legal reveal/search effect.

## Run exact-state policy tests

```bash
ctest --test-dir build --output-on-failure
./build/regidrago_policy_tests
```

`regidrago_policy_tests` checks that the simulator chooses the intended legal connector in constructed states, including Gladion prize recovery, Steven lock planning, Forest Seal Stone search behavior, Crispin and Earthen Vessel energy rules, JIT timing, search fallback, lock handling, entry Abilities, Active positioning, first-turn restrictions, and manual-attachment/evolution limits.

## Run aggregate smoke test

```bash
./build/regidrago_sim --trials 100000 --seed 20260705 --out results/simulation_results.csv
```

## Scope

A ready state means:

1. Active Regidrago VSTAR.
2. At least two Grass and one Fire attached.
3. A modeled Dragon payload in discard: Dragapult ex, Mega Dragonite ex, Dialga-GX, or Hisuian Goodra VSTAR.
4. In strict and matchup-flex profiles, that payload entered discard in the current player turn and remains in discard at the ready check.

See `docs/RULES_TRACEABILITY.md`, `docs/RULES_REVERIFICATION.md`, `docs/OPTIMAL_POLICY_FIXTURES.md`, `docs/TRACE_AUDIT.md`, `docs/MODEL_ASSUMPTIONS.md`, and `docs/REPORT.md`.
