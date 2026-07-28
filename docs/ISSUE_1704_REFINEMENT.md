# Issue 1704 refinement: Tapu Lele-GX source ID

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1704

The Pineco item-lock seed-1 bug and proposed T2 Professor Burnet route are valid. The report cites `sm1-60` for Tapu Lele-GX, while that card ID belongs to Hypno in the supplied corpus.

The modeled Tapu Lele-GX with Wonder Tag is `sm2-60`:

https://api.pokemontcg.io/v2/cards/sm2-60

The corrected route remains:

1. Bench Tapu Lele-GX on T1 and use Wonder Tag for Professor Burnet.
2. Play Earthen Vessel before Steven's Resolve, pay a realistic discard cost, search Fire Energy, and manually attach Fire to Regidrago V before the scheduled T2 Item lock.
3. Play Steven's Resolve for the remaining Forretress ex.
4. On T2, evolve Pineco, use Exploding Energy for two Grass Energy, evolve Regidrago VSTAR, and play Professor Burnet for the current-turn strict-JIT payload.
5. Reach `READY` on T2.

Supporting sources:

- Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
- Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
- Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
- Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
- Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
- Repository scheduled-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

This refinement changes only the authoritative card-data citation. It resets the approval count for the latest issue version to zero.
