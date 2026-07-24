# Issue 1452 validation

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1452

## Defect

The strict-DCI paired Mysterious Treasure plus Earthen Vessel planner required `going_first`, although the going-second seed 7 route reaches the same public T2 K1 state after Gladion reveals the Prize cards and exchanges for Mysterious Treasure.

Current-main reproduction before the fix:

```text
./build/regidrago_sim --simulate-this \
  --deck regidrago-shell \
  --scenario strict-jit/go-second \
  --seed 7
```

```text
first-ready turn: not ready through T5 | setup result: failure
```

## Legal route

1. T2 Mysterious Treasure discards one redundant Mega Dragonite ex and searches Regidrago V.
2. Bench Regidrago V.
3. T2 Earthen Vessel discards the second redundant Mega Dragonite ex and searches two Grass Energy.
4. Attach Grass on T2 and T3.
5. Professor Turo's Scenario returns Active Tapu Lele-GX, promotes Regidrago V, and permits Tapu Lele-GX to be replayed from hand.
6. Wonder Tag searches the deck-resident Gladion.
7. T4 attaches the held Fire Energy.
8. Gladion exchanges for the known prized Mysterious Treasure.
9. Mysterious Treasure discards the protected Hisuian Goodra VSTAR, searches Regidrago VSTAR, and the prior-turn Regidrago V evolves for strict-JIT T4 readiness.

The route uses no private post-shuffle draw identity. Every target and Prize location is known after the legal T2 searches.

## Sources

- Existing planner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1167_treasure_vessel_override.inc
- Prior confirmed issue: https://github.com/FlareZ123/pokemon-sims/issues/1167
- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
- Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
- Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
- Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
- Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
- Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
- Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
- Knowledge states: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
- Earliest-ready priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

## Fix scope

The fix removes only the turn-order restriction. Strict JIT, no-lock state, T2 timing, used Supporter, unused manual attachment, Active attachment-free Tapu Lele-GX, open Bench, K1 knowledge, exact held cards, searchable targets, known prized Treasure, two redundant copies, one distinct protected payload, complete future route, and no ordinary lower-DCI cost all remain required.

## Validation contract

- exact-state positive tests for going first and going second
- existing negative boundaries for locks, incomplete targets, unknown locations, short horizon, Bench capacity, payload structure, and ordinary lower-DCI costs
- complete Release and ASan/UBSan suites
- CI-built exact seed 7 trace reaching T4
- at least two independent CI-built `--simulate-this` control traces
- regenerated canonical and paired 100,000-trial setup matrices
