# pokemon-sims

A traceable, rules-mapped goldfish simulator for the supplied Pokémon TCG Live Expanded Regidrago VSTAR deck.

## Audit status

The `fix-sims` branch replaces the earlier opaque setup simulator with a policy engine that can print a full `simulate-this` transcript. Every state-changing action in a transcript has rule IDs that map to card text or a core game procedure in [`docs/RULES_TRACEABILITY.md`](docs/RULES_TRACEABILITY.md). Six deterministic hands were manually reviewed in [`docs/TRACE_AUDIT.md`](docs/TRACE_AUDIT.md).

The model is deliberately a single-player setup model. It does not claim to resolve every Expanded card or any opponent-dependent game state.

## Run one readable hand

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
./build/regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 6 --require-ready-by 3
```

The command prints the opening player-known state, debug-only prizes, every draw, card cost, search, attachment, evolution, retreat, VSTAR Power, attack, and ready-state check. The debug Prize list is never read by policy before a legal reveal/search effect.

Available scenarios include:

```text
strict-jit/go-first
strict-jit/go-second
matchup-flex-jit/go-first
matchup-flex-jit/go-second
no-discard-control/go-first
no-discard-control/go-second
strict-jit-turn2-item-lock/go-first
strict-jit-turn2-item-lock/go-second
strict-jit-full-item-lock/go-first
strict-jit-full-item-lock/go-second
strict-jit-rulebox-ability-lock/go-first
strict-jit-rulebox-ability-lock/go-second
strict-jit-combined-lock/go-first
strict-jit-combined-lock/go-second
```

Use `--find-ready 5 --start-seed 1` to print the first five seeds that reach the selected scenario’s ready condition.

## Run regression traces and aggregate smoke test

```bash
ctest --test-dir build --output-on-failure
./build/regidrago_sim --trials 100000 --seed 20260705 --out results/simulation_results.csv
```

The CTest suite includes six deterministic trace regressions across strict JIT, matchup-flex JIT, no-control, Item-lock, and Rule Box-lock configurations.

## Scope

A ready state means:

1. Active Regidrago VSTAR.
2. At least two Grass and one Fire attached.
3. A modeled Dragon payload in discard: Dragapult ex, Mega Dragonite ex, Dialga-GX, or Hisuian Goodra VSTAR.
4. In strict and matchup-flex profiles, that payload entered discard in the current player turn.

See `docs/RULES_TRACEABILITY.md`, `docs/TRACE_AUDIT.md`, `docs/MODEL_ASSUMPTIONS.md`, and `docs/REPORT.md`.
