# Issue 1645 second refinement evidence

The refined seed-218 T3 line is illegal under the authoritative Crispin ruling. When Crispin finds only one Basic Energy type, that Energy is put into hand and cannot be attached by Crispin. The current simulator implements that ruling in `play_crispin()`.

Authoritative ruling: https://compendium.pokegym.net/category/5-trainers/crispin/

Crispin card data: https://api.pokemontcg.io/v2/cards/sv7-133

Current implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011.inc#L74-L107

Related confirmed ruling issue: https://github.com/FlareZ123/pokemon-sims/issues/1461

In seed 218, two of the recipe's three Fire Energy are known Prize cards. Earthen Vessel takes the only Fire remaining in deck on T2. On T3, the recovered Crispin can therefore find only Grass Energy, which goes to hand and attaches none. One manual attachment cannot move the Regidrago VSTAR from `G` to `GGF` that turn.

The Steven target-selection defect remains valid with a T4 completion:

1. T1 Steven's Resolve searches Latias ex, Gladion, and Grass Energy.
2. T2 Bench Latias ex, evolve the prior-turn Regidrago V, use Earthen Vessel for Grass and the only deck Fire, manually attach Grass, and use Gladion to recover a prized Crispin.
3. T3 play Crispin, put the single searchable Grass into hand, attach that Grass manually, and preserve Brilliant Blender plus the held Fire.
4. T4 manually attach Fire, play Brilliant Blender for the current-turn matchup-flex-JIT payload, use Skyliner to retreat the Basic Active, and reach readiness.

Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

Gladion: https://api.pokemontcg.io/v2/cards/sm4-95

Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163

Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76

Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164

Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136

This second refinement preserves the Steven package defect while replacing the impossible T3 claim with the deterministic legal T4 route. It resets the approval count again.