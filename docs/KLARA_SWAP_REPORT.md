# Klara swap report

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1773

Klara source: https://api.pokemontcg.io/v2/cards/swsh6-145

Roseanne's Backup comparison source: https://api.pokemontcg.io/v2/cards/swsh9-148

Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

## Scope

Only `regidrago-shell` contained Roseanne's Backup on the source-bound baseline. `regidrago-pineco` contained neither Roseanne's Backup nor Klara and is unchanged, so this enhancement requires two matched matrices: shell before and shell after.

## Method

The final report records matched Release simulations from the same seed, trial count, scenario registry, and baseline commit. Exact matrices, deltas, hashes, and any negative-delta investigation are committed after the final Klara Release artifact is generated.

## Pre-final regression review

The first Release matrix was positive overall but exposed two exact strict-JIT seeds where Klara recovered a Dragon before an already-live Wonder Tag route could fetch Crispin or Tate & Liza. Those seed-301 and seed-759 losses were selector errors rather than an inherent card tradeoff. The implementation now projects the remainder of the current turn with Klara disabled and holds Klara whenever the non-Klara connector chain already completes every setup axis that turn. Both exact states are permanent regression tests. Final statistics below are generated only after that correction.
