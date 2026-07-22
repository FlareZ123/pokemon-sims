# Issue 1333 validation

The canonical Pokémon TCG Live Expanded deck must identify the original Guardians Rising Tapu Lele-GX print. Pokémon Support states that Sun & Moon cards are playable in Pokémon TCG Live while Celebrations Classic Collection cards remain unavailable for play:

https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online

The exact card-text source used by the corrected deck audit is:

https://api.pokemontcg.io/v2/cards/sm2-60

The supplied card corpus confirms that this print has Wonder Tag, the modeled attacks and Pokémon-GX rule, and a one-Colorless Retreat Cost. The correction changes source identity only. It does not change simulator transitions or setup probabilities.

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1333
