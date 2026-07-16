# Audit Corrections

## Serena draw mode

The exact supplied Serena print is `swsh12-164`:

- https://api.pokemontcg.io/v2/cards/swsh12-164

Its draw mode requires the player to **discard at least 1 card** and permits up to 3 before drawing until the player has five cards in hand.

An interim zero-discard Serena policy was identified during the rules audit and immediately removed. The active `play_serena` implementation requires a legal discard, and its strict-JIT policy may use up to three already-safe discards. Any older wording that described zero discards as legal is superseded by this correction.

## Forest Seal Stone and Rule Box lock

Forest Seal Stone remains usable while Path to the Peak is in play. The TPCi Rules Team ruling dated 2022-11-10 explains that Star Alchemy is an Ability the attached Pokémon has access to from the Tool, rather than an Ability the Pokémon has, so Path to the Peak does not suppress it:

- https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- https://api.pokemontcg.io/v2/cards/swsh12-156
- https://api.pokemontcg.io/v2/cards/swsh6-148

This interaction is a verified ruling and may be treated as a tournament-rules claim. The simulator's active Forest Seal Stone behavior under the modeled Path-style Rule Box Ability lock is consistent with that ruling.

## CI observability

The current GitHub Actions workflow now prints CTest's `LastTest.log` on failure and uploads both Release and sanitizer `Testing` directories as artifacts. The connector available during this audit cannot retrieve the prior branch push-run history or job logs, so those historical failures could not be diagnosed from their logs here.
