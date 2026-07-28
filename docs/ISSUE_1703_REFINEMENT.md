# Issue 1703 refinement: Dragapult ex source ID

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1703

The seed-38 bug and proposed T2 route are valid. The report's Dragapult ex source URL uses the nonexistent card ID `sv6pt5-130`.

The supplied Pokémon TCG card corpus contains the modeled Dragapult ex as `sv6-130`:

https://api.pokemontcg.io/v2/cards/sv6-130

The corrected route remains:

1. Play Earthen Vessel before Steven's Resolve, discard setup-inert Path to the Peak, search Grass plus Fire Energy, and manually attach Grass to the prior-turn Active Regidrago V.
2. Play Steven's Resolve for Regidrago VSTAR, Dragapult ex, and Mysterious Treasure.
3. On T2, evolve, use Crispin for the second Grass plus Fire in hand, manually attach Fire, then use Mysterious Treasure to discard Dragapult ex during the strict-JIT ready turn.
4. Reach `READY` on T2.

Supporting sources:

- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
- Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
- Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
- Repository decision policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

This refinement changes only the authoritative card-data citation. It resets the approval count for the latest issue version to zero.
