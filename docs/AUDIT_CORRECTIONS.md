# Audit Corrections

## Serena draw mode

The exact supplied Serena print is `swsh12-164`:

- https://api.pokemontcg.io/v2/cards/swsh12-164

Its draw mode requires the player to **discard at least 1 card** and permits up to 3 before drawing until the player has five cards in hand.

An interim zero-discard Serena policy was identified during the rules audit and immediately removed. The active `play_serena` implementation requires a legal discard, and its strict-JIT policy may use up to three already-safe discards. Any older wording that described zero discards as legal is superseded by this correction.

## Forest Seal Stone and Rule Box lock

The simulator still labels Forest Seal Stone activation under a modeled Rule Box Ability lock as a model assumption. The audit did not locate a direct published ruling source for that precise interaction. It must not be treated as a verified tournament claim until an official ruling is added.

## CI observability

The current GitHub Actions workflow now prints CTest's `LastTest.log` on failure and uploads both Release and sanitizer `Testing` directories as artifacts. The connector available during this audit cannot retrieve the prior branch push-run history or job logs, so those historical failures could not be diagnosed from their logs here.
