# Rules and Interaction Register

## Evidence standard and scope

This register separates three evidence classes:

1. **Core procedure** follows the official Pokémon Trading Card Game rules and resources: https://www.pokemon.com/us/pokemon-tcg/rules/
2. **Card text** is checked against the supplied `pokemon-tcg-data-master` corpus and cited with the exact modeled print where a model decision depends on it.
3. **Pokémon TCG Live Expanded availability** remains a client-side assumption. Paper card-pool data does not prove that a print is currently available in the Live client.

The simulator models the setup path through the first legal `Apex Dragon` attack. It is a policy simulator, not a full opponent or combat engine.

## Core game procedure

### Setup and opening sequence

1. Shuffle the 60-card deck and draw seven cards.
2. A player with no Basic Pokémon reveals a mulligan, shuffles, and draws a new seven-card hand. The opponent may draw for each opposing mulligan.
3. Choose a Basic Active Pokémon and optionally Bench Basic Pokémon from that opening hand.
4. Put six Prize cards face down from the remaining deck.
5. Determine turn order and begin the game.

The opening hand is therefore selected before Prize cards are placed. The simulator takes six exact Prizes after Active and opening-Bench selection, without an additional shuffle: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play

At the start of every turn, including the first player’s first turn, the player draws one card. The first-player restriction applies to Supporters and attacks: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Start_Your_Turn https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn

### First-turn and turn limits

- The player going first cannot play a Supporter or attack on that player’s first turn: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#First_Turn
- A player may attach one Energy from hand each turn. Effects can attach additional Energy.
- A player may play one Supporter each turn and any number of Items when no effect prevents them.
- A Pokémon cannot evolve during the player’s first turn or during the turn that Pokémon entered play.
- The Bench limit is five Pokémon.
- Retreat is limited to once per turn. A retreat does not use the manual Energy attachment.

## Hidden-information policy

Before a legal search or Prize reveal, the policy uses only public zones, cards in hand, board state, and known deck copy counts. It does not select actions from unseen draw order.

A legal deck search records K1 knowledge for future policy decisions. Hisuian Heavy Ball can reveal Prize cards and exchange itself for a revealed Basic Pokémon: https://api.pokemontcg.io/v2/cards/swsh10-146. Once the deck or all Prizes have been legally observed, the fixed 60-card list and known zones make remaining deck identities deducible for this model.

This K0/K1 boundary is a policy discipline. It does not change card text.

## Rule Box and Tool interactions

### Path to the Peak model

Path to the Peak gives Pokémon with a Rule Box no Abilities: https://api.pokemontcg.io/v2/cards/swsh6-148

The modeled Rule Box lock suppresses Rule Box Pokémon Abilities, including Tapu Lele-GX’s Wonder Tag, Latias ex’s Skyliner, Mawile-GX’s Captivating Wink, and Regidrago VSTAR’s Legacy Star. It does not stop attacks, so Apex Dragon remains usable.

Oricorio GRI 55 has no Rule Box. Its Vital Dance remains usable through this Path-style lock when it is played from hand to Bench during a turn: https://api.pokemontcg.io/v2/cards/sm2-55

### Forest Seal Stone through Path to the Peak

Forest Seal Stone is a Pokémon Tool. Its text allows the attached Pokémon V to use the VSTAR Power printed on the Tool: https://api.pokemontcg.io/v2/cards/swsh12-156

The modeled Path lock removes Abilities on Rule Box Pokémon. It does not remove text printed on an attached Pokémon Tool. An already attached Forest Seal Stone may therefore use Star Alchemy through the modeled Rule Box lock. Pokémon Tools are separate from Item cards after the Tool errata, so the model also allows Forest Seal Stone to be attached from hand during Item lock; the game-wide one-VSTAR-Power limit remains shared with Legacy Star: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata

## Modeled cards and interactions

| Card | Modeled consequence | Source |
|---|---|---|
| Regidrago V | Celestial Roar is modeled on any legal attack turn after other setup actions. It requires one attached Energy, processes the top three cards, and attaches any Energy among them to itself. | https://api.pokemontcg.io/v2/cards/swsh12-135 |
| Regidrago VSTAR | Apex Dragon needs GGF and uses an attack from a Dragon Pokémon in discard. Legacy Star shares the game-wide VSTAR Power limit. | https://api.pokemontcg.io/v2/cards/swsh12-136 |
| Forest Seal Stone | Attached Pokémon V can use Tool-printed Star Alchemy. The model allows it to be attached during Item lock because Pokémon Tools are separate from Item cards after the Tool errata. | https://api.pokemontcg.io/v2/cards/swsh12-156 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata |
| Chaotic Swell | Replaces the modeled Path to the Peak and restores Rule Box Pokémon Abilities. Its replacement-triggered discard of the incoming Stadium is outside the one-Path removal route modeled here. | https://api.pokemontcg.io/v2/cards/sm12-187 |
| Powerglass | Attaches one Basic Energy from discard to its own Active holder at the end of the turn, after the attack timing point. That Energy can contribute to readiness on a later turn. | https://api.pokemontcg.io/v2/cards/sv6pt5-63 |
| Mysterious Treasure | Discard one card from hand to search for a Psychic or Dragon Pokémon. | https://api.pokemontcg.io/v2/cards/sm6-113 |
| Dipplin | A Dragon Pokémon, therefore a legal Mysterious Treasure target and fallback. Maintainer policy excludes Syrup Catcher from the A/S payload set, readiness predicate, and payload-discard routes. | https://api.pokemontcg.io/v2/cards/sv6-127 |
| Quick Ball | Discard another card from hand to search for a Basic Pokémon. | https://api.pokemontcg.io/v2/cards/swsh1-179 |
| Earthen Vessel | Discard another card from hand to search for up to two Basic Energy. Same-type selections are legal. | https://api.pokemontcg.io/v2/cards/sv4-163 |
| Brilliant Blender | Search the deck for up to five cards and discard them. It discards from deck rather than hand. | https://api.pokemontcg.io/v2/cards/sv8-164 |
| Professor Burnet | Search the deck for up to two cards and discard them. It is the modeled Item-lock payload bridge only while a payload may remain in deck. | https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 |
| Crispin | Search for up to two different-type Basic Energy, put one into hand, and attach the other. A one-type resolution puts the found Energy into hand and attaches none. | https://api.pokemontcg.io/v2/cards/sv7-133 |
| Arven | Search for an Item and a Pokémon Tool. The policy treats a Tool-only Forest Seal Stone route as live when it can repair a current axis, including while Item lock blocks Arven’s Item target. | https://api.pokemontcg.io/v2/cards/sv1-166 https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata |
| Steven’s Resolve | Search for up to three cards, then end the turn. The selection re-evaluates after that legal deck inspection. | https://api.pokemontcg.io/v2/cards/sm7-145 |
| Gladion | Inspect Prize cards, take one, then shuffle Gladion into the remaining Prizes. | https://api.pokemontcg.io/v2/cards/sm4-95 |
| Serena | In draw mode, discard up to three cards, but the print requires at least one discard; then draw until five cards are in hand. | https://api.pokemontcg.io/v2/cards/swsh12-164 |
| Tate & Liza | Choose shuffle-and-draw-five or switch Active with a Benched Pokémon. | https://api.pokemontcg.io/v2/cards/sm7-148 |
| Tapu Lele-GX | Wonder Tag triggers only when played from hand to Bench during a turn and is blocked by the Rule Box lock. | https://api.pokemontcg.io/v2/cards/sm2-60 |
| Latias ex | Skyliner gives Basic Pokémon in play no Retreat Cost. It does not make a Regidrago VSTAR itself free to retreat. | https://api.pokemontcg.io/v2/cards/sv8-76 |
| Oricorio | Vital Dance searches for up to two Basic Energy when played from hand to Bench during a turn. It is not blocked by the modeled Rule Box lock. | https://api.pokemontcg.io/v2/cards/sm2-55 |

## Strict JIT policy

Strict and matchup-flex JIT profiles require an A- or S-tier Dragon payload to enter discard during the same turn as the ready-state check. The current model accepts Dragapult ex and Mega Dragonite ex as S-tier payloads, plus Dialga-GX and Hisuian Goodra VSTAR as A-tier payloads.

Dipplin TWM 127 is deliberately outside that A/S set. This maintainer policy treats Syrup Catcher as an unreliable game-winning line: it may take no Prize, and the opponent can switch after the effect. Dipplin can remain a legal Dragon search target and a discard-cost card without satisfying the ready-state predicate. https://api.pokemontcg.io/v2/cards/sv6-127

The payload may enter discard through a legal hand-discard or deck-discard effect. The model distinguishes those routes because Mysterious Treasure, Quick Ball, Earthen Vessel, and Serena can discard from hand, while Brilliant Blender and Professor Burnet can only discard cards they search from deck.

After K1 proves that no payload remains in deck, the policy preserves a dead Blender or Burnet route and uses a live hand-discard route when one exists. Before K1, the policy does not assume that the payload is absent.

## DCI, AMR, and connector policy

The simulator uses discard-capable categories instead of treating every card in hand as equal fuel. Strict JIT protects key singletons, recovery, matchup cards, and payloads before the payload turn. Matchup-flex admits selected low-value matchup cards. No-discard-control permits earlier payload banking.

Connector policy weighs directness, cost, Supporter contention, Item lock, Ability lock, Bench capacity, prize information, and whether a card can satisfy a missing Regidrago, VSTAR, Energy, Active-position, or current-turn payload axis.

## Deliberate bounds

The model does not simulate an opposing deck, damage race, Knock Outs, prize-taking, hand disruption, gust resolution, Bench pressure, post-setup recovery loops, general Field Blower target selection, arbitrary Stadium sequencing beyond the modeled Path-to-Chaotic-Swell removal, or full copied-attack combat effects. Lock scenarios are injected constraints.

The reported percentages are setup-readiness policy estimates under those stated limits. They are neither match-win predictions nor exhaustive proofs of optimal play.
