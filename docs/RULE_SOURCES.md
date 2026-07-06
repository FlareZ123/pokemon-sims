# Rule and Card-Text Sources

## Source policy

Every modeled game mechanic is tied to a direct source URL. Card-specific behavior uses the exact print's JSON record in the Pokémon TCG API, which is also the identifier used by the supplied card-data corpus. Core game procedures use the official Pokémon TCG rules landing page.

- Core procedure reference: https://www.pokemon.com/us/pokemon-tcg/rules
- The simulator's direct raw URLs are placed beside the relevant implementation methods as `// Card text source:` or `// Core procedure source:` comments.

The API records are card-text sources. They do not replace official tournament rulings. Where the model uses a policy choice, it is marked `P-*` in traces and described separately in `POLICY_DECISIONS.md`.

## Core procedure sources

| Mechanic | Direct source | Implemented in |
|---|---|---|
| Setup, opening hand, six Prize cards, turn draw, first-turn limits, Bench, manual attachment, Supporter limit, Item timing, evolution, retreat | https://www.pokemon.com/us/pokemon-tcg/rules | `setup`, `begin_turn`, `bench_from_hand`, `attach_manual`, `supporter_allowed`, `item_locked`, `evolve_best_regi`, retreat helpers |
| One VSTAR Power in a game | https://api.pokemontcg.io/v2/cards/swsh12-136 and https://api.pokemontcg.io/v2/cards/swsh12-156 | `vstar_power_used`, `use_fss`, `use_legacy_star` |

## Exact card-print sources

| Model card / effect | Exact card-text URL |
|---|---|
| Regidrago V — Celestial Roar | https://api.pokemontcg.io/v2/cards/swsh12-135 |
| Regidrago VSTAR — Apex Dragon and Legacy Star | https://api.pokemontcg.io/v2/cards/swsh12-136 |
| Forest Seal Stone — Star Alchemy | https://api.pokemontcg.io/v2/cards/swsh12-156 |
| Tapu Lele-GX — Wonder Tag | https://api.pokemontcg.io/v2/cards/cel25c-60_A |
| Oricorio GRI 55 — Vital Dance | https://api.pokemontcg.io/v2/cards/sm2-55 |
| Latias ex — Skyliner | https://api.pokemontcg.io/v2/cards/sv8-76 |
| Hisuian Heavy Ball | https://api.pokemontcg.io/v2/cards/swsh10-146 |
| Mysterious Treasure | https://api.pokemontcg.io/v2/cards/sm6-113 |
| Quick Ball | https://api.pokemontcg.io/v2/cards/swsh1-179 |
| Ultra Ball comparator | https://api.pokemontcg.io/v2/cards/swsh12pt5-146 |
| Evolution Incense comparator | https://api.pokemontcg.io/v2/cards/swsh1-163 |
| Earthen Vessel | https://api.pokemontcg.io/v2/cards/sv4-163 |
| Brilliant Blender | https://api.pokemontcg.io/v2/cards/sv8-164 |
| Arven | https://api.pokemontcg.io/v2/cards/sv1-166 |
| Crispin | https://api.pokemontcg.io/v2/cards/sv7-133 |
| Professor Burnet | https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 |
| Serena | https://api.pokemontcg.io/v2/cards/swsh12-164 |
| Tate & Liza | https://api.pokemontcg.io/v2/cards/sm7-148 |
| Steven's Resolve | https://api.pokemontcg.io/v2/cards/sm7-145 |
| Gladion | https://api.pokemontcg.io/v2/cards/sm4-95 |
| Lusamine | https://api.pokemontcg.io/v2/cards/sm4-96 |
| Path to the Peak | https://api.pokemontcg.io/v2/cards/swsh6-148 |
| Dragapult ex payload model | https://api.pokemontcg.io/v2/cards/sv6-130 |
| Mega Dragonite ex payload model | https://api.pokemontcg.io/v2/cards/me2pt5-152 |
| Dragon Dialga-GX payload model | https://api.pokemontcg.io/v2/cards/sm5-100 |
| Hisuian Goodra VSTAR payload model | https://api.pokemontcg.io/v2/cards/swsh11-136 |

## Verified versus modeled interactions

- Direct card-text mechanics in this table are validated against the supplied card corpus and the linked exact-print record.
- The `FullRuleBoxAbility` scenario is a model abstraction for a Path-to-the-Peak-style lock. It suppresses Rule Box Pokémon Abilities and leaves Oricorio Vital Dance available because Oricorio GRI 55 has no Rule Box.
- Forest Seal Stone under the Rule Box lock remains an explicit model assumption. The simulator treats an already attached Tool's Star Alchemy as distinct from playing an Item from hand. Do not interpret this as a published tournament ruling until a direct official ruling source is added here.
- Ultra Ball and Evolution Incense are comparators only, not cards in the submitted baseline deck.
