# Issue 1645 third refinement evidence

The latest-main patched trace proves that the previously proposed Gladion-to-prized-Crispin bridge is legal but unnecessary. Steven's Resolve can search **up to** three cards, and the deterministic optimal T4 package is only:

- Latias ex
- Grass Energy

The complete route is:

1. T1 Steven's Resolve searches Latias ex and Grass Energy, then ends the turn.
2. T2 manually attach the searched Grass to the prior-turn Regidrago V, evolve with the held Regidrago VSTAR, Bench Latias ex, and play the held Earthen Vessel. Vessel discards the held Mega Dragonite ex and searches the remaining Grass plus the only Fire Energy left in deck. Skyliner then gives the Basic Active Oricorio no Retreat Cost, allowing promotion of Regidrago VSTAR.
3. T3 manually attach the Grass searched by Earthen Vessel, reaching `GG`.
4. T4 manually attach the held Fire, play the held Brilliant Blender for a current-turn matchup-flex-JIT Dragon payload, and reach readiness.

Searching Gladion as a third Steven target and later exchanging it for Crispin cannot improve the T4 ready turn. It consumes the singleton Prize-exchange resource and the T3 Supporter permission while the direct manual-attachment route already completes all setup axes. Under the repository's DCI, discrete-value, and earliest-route policy, the Gladion-Crispin bridge is therefore weaker than searching only Latias ex and Grass Energy.

Supporting sources:

- Steven's Resolve searches up to three cards: https://api.pokemontcg.io/v2/cards/sm7-145
- Earthen Vessel searches up to two Basic Energy after discarding another card: https://api.pokemontcg.io/v2/cards/sv4-163
- Latias ex gives Basic Pokémon no Retreat Cost: https://api.pokemontcg.io/v2/cards/sv8-76
- Brilliant Blender searches and discards up to five cards: https://api.pokemontcg.io/v2/cards/sv8-164
- Regidrago VSTAR evolves from Regidrago V and Apex Dragon costs `GGF`: https://api.pokemontcg.io/v2/cards/swsh12-136
- Core Supporter, Item, attachment, evolution, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
- K1, DCI, discrete-value preservation, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
- Issue: https://github.com/FlareZ123/pokemon-sims/issues/1645
- Source-bound diagnostic PR: https://github.com/FlareZ123/pokemon-sims/pull/1666

This third refinement preserves the original target-selection defect while replacing the resource-negative Gladion-Crispin continuation with the deterministic direct Latias-Grass T4 route. It resets the approval count to zero.
