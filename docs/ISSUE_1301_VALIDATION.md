# Issue 1301 validation

Mawile-GX `sm11-141` has a printed Retreat Cost of one Colorless Energy. The simulator now records that cost, preventing Exploding Energy from attaching only two Grass to Regidrago and then promoting through an Active Mawile-GX for free.

Sources:

- https://api.pokemontcg.io/v2/cards/sm11-141
- https://api.pokemontcg.io/v2/cards/sv4pt5-2
- https://www.pokemon.com/us/pokemon-tcg/rules
- https://github.com/FlareZ123/pokemon-sims/issues/1301

The permanent regression checks all modeled printed Retreat Costs and a reachable Exploding Energy fixture. The final branch evidence was regenerated from current `main` with 100,000 trials per matrix condition and no temporary lock artifacts. Repository CI also validates the canonical shell matrix and the paired shell/Pineco matrix against the same source digest.
