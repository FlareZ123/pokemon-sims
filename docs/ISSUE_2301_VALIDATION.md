# Issue 2301 deterministic route validation

Issue: https://github.com/FlareZ123/pokemon-sims/issues/2301

Pull request: https://github.com/FlareZ123/pokemon-sims/pull/2305

The source-bound regression replays `regidrago-pineco`, `strict-jit/go-second`, seed `38` and requires T4 readiness through the K1-proven route. T3 Quick Ball establishes Regidrago V and the evolution timer while preserving Earthen Vessel, Secret Box, Crispin, Regidrago VSTAR, and a second Dragon. T4 Earthen Vessel discards that Dragon as the strict-JIT payload, Secret Box spends only route-replaced known cards, Forest Seal Stone replaces the spent VSTAR, Dawn plus Forest of Vitality produces Forretress ex, Exploding Energy supplies the two Grass attachments, manual Fire completes GGF, and Tapu Lele-GX pays its Retreat Cost before Regidrago VSTAR is promoted.

Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
Dawn: https://api.pokemontcg.io/v2/cards/me2-87
Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
Official rulebook: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
Knowledge states: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
