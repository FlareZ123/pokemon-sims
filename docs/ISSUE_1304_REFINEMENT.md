# Issue 1304 refined route

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1304

The original T3 Professor Burnet proposal is not source-bound. Using Star Alchemy on turn one adds a second shuffle and changes the cited turn-two draw from Fire Energy to Mysterious Treasure. The valid route holds Star Alchemy on turn one and reaches strict-JIT readiness on turn two.

## Source-bound route

1. Turn one: play Mysterious Treasure, discard Erika's Invitation, search Regidrago V, and Bench it.
2. Attach Forest Seal Stone to Regidrago V and hold Star Alchemy.
3. Play Forest of Vitality.
4. The single Treasure shuffle produces Mysterious Treasure as the turn-two draw for seed 31415.
5. Turn two: evolve Pineco into Forretress ex and use Exploding Energy to attach Grass Energy and promote Regidrago V.
6. Use Star Alchemy to search Fire Energy and attach it.
7. Play the drawn Mysterious Treasure, discard Hisuian Goodra VSTAR as the current-turn payload, and search Regidrago VSTAR.
8. Evolve Regidrago V. The Active Regidrago VSTAR has at least GGF and a permitted Dragon payload entered discard during the same turn.

Local source-bound prototype result:

```text
Deck: regidrago-pineco
Scenario: strict-jit/go-first
Seed: 31415
first-ready turn: 2
setup result: success
```

## Sources

- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
- Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
- Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
- Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
- Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
- Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Knowledge states: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
- Strict-JIT treatment: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
- Decision priorities: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

No production change is committed on this branch while the refined issue awaits two independent approvals.