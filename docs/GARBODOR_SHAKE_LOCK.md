# Garbodor + Boost Shake Ability-Lock Scenario

`garbodor-shake-ability-lock` is a paper-Expanded opponent-pressure scenario for Garbodor BKP 57 established with Boost Shake.

- Going first, Regidrago's first turn occurs before the opponent's first turn. Pokemon Abilities are available on Regidrago T1, then Garbotoxin is active from Regidrago T2 onward.
- Going second, the opponent has already completed the Boost Shake turn, so Garbotoxin is active from Regidrago T1 onward.
- Garbotoxin suppresses Pokemon Abilities while Garbodor has a Pokemon Tool attached. It does not create an Item, Supporter, Stadium, Tool, or attachment lock.
- Forest Seal Stone remains governed by the repository's existing Tool-Ability ruling. Star Alchemy is granted by the Tool to an attached Pokemon V rather than being that Pokemon's own Ability.
- Field Blower may discard the modeled Tool from Garbodor when Items are legal. The simulator spends it only when the current-turn removal unlocks a live setup connector such as Wonder Tag, Skyliner, Vital Dance, Dark Asset, or Legacy Star.
- The opponent is modeled at maximum pressure. After a successful Field Blower turn, another Pokemon Tool is assumed attached to Garbodor before Regidrago's next turn. The unlock marker is keyed to the current player turn, so it cannot persist into the next turn.
- Garbodor's Tool state is independent of Path to the Peak and Chaotic Swell state.

Card and rule sources:

- Garbodor BKP 57 / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
- Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
- Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
- Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
- Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
- Core Pokemon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
