# Garbodor + Boost Shake Ability-Lock Scenario

`garbodor-shake-ability-lock` is a paper-Expanded opponent-pressure scenario for Garbodor BKP 57 established with Boost Shake.

- Going first, Regidrago's first turn occurs before the opponent's first turn. Pokémon Abilities are available on Regidrago T1, then Garbotoxin is active from Regidrago T2 onward.
- Going second, the opponent has already completed the Boost Shake turn, so Garbotoxin is active from Regidrago T1 onward.
- Garbotoxin suppresses Pokémon Abilities while Garbodor has a Pokémon Tool attached. It does not create an Item, Supporter, Stadium, Tool, or attachment lock by itself.
- Forest Seal Stone remains governed by the repository's existing Tool-Ability ruling. Star Alchemy is granted by the Tool to an attached Pokémon V rather than being that Pokémon's own Ability.
- Field Blower may discard the modeled Tool from Garbodor when Items are legal. The simulator spends it only when the current-turn removal unlocks a live setup connector such as Wonder Tag, Skyliner, Vital Dance, Dark Asset, or Legacy Star.
- The opponent is modeled at maximum pressure: after a successful Field Blower turn, assume another Pokémon Tool is attached to Garbodor before Regidrago's next turn. The unlock is therefore current-turn only and must not persist into the next player turn.

Card and rule sources:

- Garbodor BKP 57 / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
- Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
- Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
- Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
- Core Pokémon TCG rules: https://www.pokemon.com/us/pokemon-tcg/rules
- Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
