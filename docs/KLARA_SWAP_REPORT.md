# Klara swap report

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1773

Klara source: https://api.pokemontcg.io/v2/cards/swsh6-145

Roseanne's Backup comparison source: https://api.pokemontcg.io/v2/cards/swsh9-148

Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Scope

Only `regidrago-shell` contained Roseanne's Backup on the source-bound baseline. `regidrago-pineco` contained neither Roseanne's Backup nor Klara and is unchanged, so this enhancement requires two matched matrices: shell before and shell after.

## Method

The final report records matched Release simulations from the same seed, trial count, scenario registry, and baseline commit. Exact matrices, deltas, hashes, and any negative-delta investigation are committed after the first Klara Release artifact is generated.
