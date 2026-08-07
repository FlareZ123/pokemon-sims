# Issue 2238 deterministic audit seeds

The full audit rationale and matrix are in `docs/DOUBLE_DRAGON_ENERGY_VALIDATION.md`.

The final DDE model was manually replayed with:

- `strict-jit/go-first`, seed `3`: manual DDE compression.
- `strict-jit/go-first`, seed `4`: Basic Fire is used on the equal-finish turn, preserving DDE.
- `strict-jit/go-second`, seed `22`: Celestial Roar attaches DDE to Regidrago V.
- `strict-jit/go-first`, seed `90`: Star Alchemy searches DDE with no DDE already held.
- `matchup-flex-jit/go-first`, seed `11`: manual DDE compression followed by a payload-only Legacy Star use.

Card/rule sources:

- Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
- Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
- Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
- Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
