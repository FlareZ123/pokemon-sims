# Issue 1299 validation

The Pineco combo's paid Ultra Ball search now includes Appletun in its post-inspection fallback set. Ultra Ball may search for any Pokémon after its two-card discard cost is paid, and Appletun `sv8-140` is a Pokémon represented by the simulator.

Sources:

- https://api.pokemontcg.io/v2/cards/swsh12pt5-146
- https://api.pokemontcg.io/v2/cards/sv8-140
- https://github.com/FlareZ123/pokemon-sims/issues/1299

The permanent regression covers Appletun as the only legal Pokémon remaining in the deck and a no-Pokémon control. Branch validation also ran the complete Release and sanitizer suites, source-bound 100,000-trial matrices, and three reviewed `--simulate-this` traces before merge.
