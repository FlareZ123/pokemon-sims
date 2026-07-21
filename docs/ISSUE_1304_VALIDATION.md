# Issue 1304 validation

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1304

The Pineco seed-31415 policy uses the setup-dead Erika's Invitation as the paid T1 Mysterious Treasure cost only when the complete observable T2 route exists. The search establishes Regidrago V, Forest Seal Stone remains attached with Star Alchemy unused, prior-turn Pineco evolves into Forretress ex on T2, Exploding Energy supplies Grass, Star Alchemy searches Fire, and the source-bound second Treasure discards Hisuian Goodra VSTAR while searching Regidrago VSTAR.

Authoritative sources: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/sv3pt5-160 https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/swsh12-156 https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh11-136 https://www.pokemon.com/us/pokemon-tcg/rules

Validation covers the exact full seed, route admission, missing cost, missing Regidrago V, missing Forest Seal Stone, missing Forretress ex, missing Regidrago VSTAR, missing Fire, missing Grass, Item lock, Rule Box Ability lock, insufficient horizon, and going-second isolation. The evidence refresh runs three independent `--simulate-this` passes, both 100,000-trial source-bound matrices, and the complete Release suite before committing refreshed results.
