# Garbodor + Boost Shake Ability-Lock Scenario

`garbodor-shake-ability-lock` is a paper-Expanded opponent-pressure scenario for Garbodor BKP 57 established with Boost Shake.

- Going first, Regidrago's first turn occurs before the opponent's first turn. Pokemon Abilities are available on Regidrago T1, then Garbotoxin is active from Regidrago T2 onward.
- Going second, the opponent has already completed the Boost Shake turn, so Garbotoxin is active from Regidrago T1 onward.
- Garbotoxin suppresses Pokemon Abilities while Garbodor has a Pokemon Tool attached. It does not create an Item, Supporter, Stadium, Tool, or attachment lock by itself.
- Forest Seal Stone remains governed by the repository's existing Tool-Ability ruling. Star Alchemy is granted by the Tool to an attached Pokemon V rather than being that Pokemon's own Ability.
- Field Blower may discard the modeled Tool from Garbodor when Items are legal. The simulator spends it only when the current-turn removal unlocks a live setup connector.
- **Tiny lock-scenario note:** Field Blower's Garbodor relief lasts for the current player turn only. For maximum-pressure modeling, assume the opponent has another Tool ready and Garbotoxin is active again before Regidrago's next turn.
- Arven may search Field Blower only when that current-turn unlock enables a usable non-Supporter Ability route. Wonder Tag alone is not enough because Arven has already consumed the turn's Supporter action.
- Chaotic Swell does not answer Garbotoxin. Removing or replacing a Stadium does not remove Garbodor's attached Tool.

Card and rule sources:

- Garbodor BKP 57 / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
- Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
- Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
- Arven: https://api.pokemontcg.io/v2/cards/sv1-166
- Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
- Core Pokemon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
