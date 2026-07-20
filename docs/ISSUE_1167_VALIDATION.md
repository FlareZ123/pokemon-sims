# Issue 1167 Integrated Validation

The exact-state regression is validated together with the prerequisite Turo and Wonder Tag fixes from issues https://github.com/FlareZ123/pokemon-sims/issues/1165 and https://github.com/FlareZ123/pokemon-sims/issues/1166.

The paid-search route follows Mysterious Treasure https://api.pokemontcg.io/v2/cards/sm6-113, Earthen Vessel https://api.pokemontcg.io/v2/cards/sv4-163, and the core Pokémon TCG procedure https://www.pokemon.com/us/pokemon-tcg/rules.

The final PR CI regenerates the 100,000-trial setup matrix and runs the complete test suite plus eight independent `--simulate-this` audits against the integrated tree.
