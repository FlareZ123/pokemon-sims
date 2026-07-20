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
| Powerglass | https://api.pokemontcg.io/v2/cards/sv6pt5-63 |
| Dawn | https://api.pokemontcg.io/v2/cards/me2-87 |
| Forest of Vitality | https://api.pokemontcg.io/v2/cards/me1-117 |
| Pineco | https://api.pokemontcg.io/v2/cards/sv4pt5-1 |
| Forretress ex — Exploding Energy | https://api.pokemontcg.io/v2/cards/sv4pt5-2 |
| Tapu Lele-GX — Wonder Tag | https://api.pokemontcg.io/v2/cards/cel25c-60_A |
| Oricorio GRI 55 — Vital Dance | https://api.pokemontcg.io/v2/cards/sm2-55 |
| Latias ex — Skyliner | https://api.pokemontcg.io/v2/cards/sv8-76 |
| Erika's Invitation — opponent-dependent effect is deliberately inert in the single-player model | https://api.pokemontcg.io/v2/cards/sv3pt5-160 |
| Hisuian Heavy Ball | https://api.pokemontcg.io/v2/cards/swsh10-146 |
| Mysterious Treasure | https://api.pokemontcg.io/v2/cards/sm6-113 |
| Quick Ball | https://api.pokemontcg.io/v2/cards/swsh1-179 |
| Ultra Ball comparator | https://api.pokemontcg.io/v2/cards/swsh12pt5-146 |
| Evolution Incense comparator | https://api.pokemontcg.io/v2/cards/swsh1-163 |
| Pokémon Communication variant support | https://api.pokemontcg.io/v2/cards/sm9-152 |
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
| Chaotic Swell | https://api.pokemontcg.io/v2/cards/sm12-187 |
| Field Blower | https://api.pokemontcg.io/v2/cards/sm2-125 |
| Team Yell's Cheer | https://api.pokemontcg.io/v2/cards/swsh9-149 |
| Roseanne's Backup | https://api.pokemontcg.io/v2/cards/swsh9-148 |
| Professor Turo's Scenario | https://api.pokemontcg.io/v2/cards/sv4-171 |
| Dragapult ex payload model | https://api.pokemontcg.io/v2/cards/sv6-130 |
| Mega Dragonite ex payload model | https://api.pokemontcg.io/v2/cards/me2pt5-152 |
| Dragon Dialga-GX payload model | https://api.pokemontcg.io/v2/cards/sm5-100 |
| Hisuian Goodra VSTAR payload model | https://api.pokemontcg.io/v2/cards/swsh11-136 |

## Recent policy-fix traceability

| Issue | Policy boundary | Direct sources | Implementation |
|---|---|---|---|
| https://github.com/FlareZ123/pokemon-sims/issues/1067 | Prefer the current-turn Arven, Forest Seal Stone, Regidrago VSTAR, and Brilliant Blender finish over Steven's Resolve ending the turn | https://api.pokemontcg.io/v2/cards/sv1-166, https://api.pokemontcg.io/v2/cards/swsh12-156, https://api.pokemontcg.io/v2/cards/swsh12-136, https://api.pokemontcg.io/v2/cards/sv8-164, https://api.pokemontcg.io/v2/cards/sm7-145 | `part_issue_1067_arven_before_late_steven_override.inc`, `part_issue_1030_steven_turo_override.inc` |
| https://github.com/FlareZ123/pokemon-sims/issues/1068 | Allocate Star Alchemy to Latias ex when Earthen Vessel covers the final Energy and payload cost | https://api.pokemontcg.io/v2/cards/swsh12-156, https://api.pokemontcg.io/v2/cards/sv8-76, https://api.pokemontcg.io/v2/cards/sv4-163, https://api.pokemontcg.io/v2/cards/swsh12-136 | `part_issue_1071_fss_oricorio_treasure_decomposition_override.inc`, `part_011_fss_latias_override.inc` |
| https://github.com/FlareZ123/pokemon-sims/issues/1069 | Admit Legacy Star when one resolution must supply both the manual-attachment Energy and the strict-JIT payload | https://api.pokemontcg.io/v2/cards/swsh12-136, https://api.pokemontcg.io/v2/cards/sv8-164 | `part_issue_1069_legacy_star_combined_energy_payload_override.inc` |
| https://github.com/FlareZ123/pokemon-sims/issues/1070 | Preserve Tate & Liza switch mode after Star Alchemy searches the VSTAR for a prepared Benched Basic | https://api.pokemontcg.io/v2/cards/sm7-148, https://api.pokemontcg.io/v2/cards/swsh12-156, https://api.pokemontcg.io/v2/cards/swsh12-136, https://api.pokemontcg.io/v2/cards/sv8-164 | `part_issue_1070_tate_after_vstar_search_override.inc`, `part_tate_blender_tate_override.inc` |
| https://github.com/FlareZ123/pokemon-sims/issues/1071 | Let Mysterious Treasure cover the VSTAR axis while Star Alchemy covers Oricorio and Vital Dance Energy | https://api.pokemontcg.io/v2/cards/swsh12-156, https://api.pokemontcg.io/v2/cards/sm6-113, https://api.pokemontcg.io/v2/cards/sm2-55, https://api.pokemontcg.io/v2/cards/swsh12-136, https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 | `part_issue_1071_fss_oricorio_treasure_decomposition_override.inc` |
| https://github.com/FlareZ123/pokemon-sims/issues/1079 | Hold Celestial Roar when one held Basic Energy and the next legal manual attachment already guarantee GGF before evolution, while strict JIT cannot bank an early payload | https://api.pokemontcg.io/v2/cards/swsh12-135, https://api.pokemontcg.io/v2/cards/swsh12-136, https://www.pokemon.com/us/pokemon-tcg/rules | `part_celestial_roar_override.inc`, `run_issue_1079_celestial_roar_hold.cmake` |

All six boundaries also cite the official turn, Trainer, evolution, attachment, Ability, attack, and Retreat procedures at https://www.pokemon.com/us/pokemon-tcg/rules and the repository's earliest-complete-route and strict-JIT specifications in `POLICY_DECISIONS.md` and `MODEL_ASSUMPTIONS.md`.

## Verified versus modeled interactions

- Direct card-text mechanics in this table are validated against the supplied card corpus and the linked exact-print record.
- The `FullRuleBoxAbility` scenario is a model abstraction for a Path-to-the-Peak-style lock. It suppresses Rule Box Pokémon Abilities and leaves Oricorio Vital Dance available because Oricorio GRI 55 has no Rule Box.
- Forest Seal Stone remains usable under Path to the Peak. The published ruling explains that the Pokémon V does not have Star Alchemy; it has access to the Ability printed on the attached Tool: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- Ultra Ball, Evolution Incense, and Pokémon Communication are variant-support cards only. They are not cards in the submitted baseline deck.
