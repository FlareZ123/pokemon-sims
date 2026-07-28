# Issue 1721 probability refinement

The T1 Arven, Earthen Vessel, Powerglass, Quick Ball, and Oricorio route remains a legal and strongly preferred setup line. The previously stated `94.787744%` conservative T2 readiness bound treated every non-outlet T2 draw as leaving all eleven Legacy Star success cards in the remaining 37-card deck. That is false when the T2 draw itself is one of the four Dragon payloads.

After the route, the known 38-card deck contains seven direct same-turn strict-JIT outlets and four Dragon payloads:

- Direct outlets: Quick Ball x2, Mysterious Treasure x2, Brilliant Blender x1, Professor Burnet x1, Serena x1.
- Dragon payloads: Dragapult ex x2, Mega Dragonite ex x1, Dialga-GX x1.

The conservative probability must condition on three disjoint T2 draw classes:

```text
7/38
+ 4/38 * (1 - C(27,7) / C(37,7))
+ 27/38 * (1 - C(26,7) / C(37,7))
= 700597 / 740962
= 0.9455235221
= 94.552352%
```

A direct-outlet draw succeeds immediately. A payload draw leaves ten Legacy Star success cards and 27 failures in the remaining deck. An irrelevant draw leaves eleven success cards and 26 failures. This remains conservative because it excludes additional live continuations through Arven, Tapu Lele-GX, Hisuian Heavy Ball, Crobat V, and Tate & Liza.

Card and rules sources:

- Arven: https://api.pokemontcg.io/v2/cards/sv1-166
- Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
- Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
- Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
- Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
- Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
- Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
- Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
- Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
- Observable-state and future-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
- Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
- Issue: https://github.com/FlareZ123/pokemon-sims/issues/1721
