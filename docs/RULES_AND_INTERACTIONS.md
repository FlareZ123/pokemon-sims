# Rules and Interaction Register

## Evidence standard and scope

This register distinguishes three evidence classes.

1. **Rulebook-level rules** are taken from the official Pokémon Trading Card Game rulebook and the Play! Pokémon rules and resources page.
2. **Card text** is validated against `data/card_audit.json`, extracted from the supplied `pokemon-tcg-data-master` corpus.
3. **Live-client availability** is an input assumption from the supplied deck list. The card corpus records paper legality, not the dynamic Pokémon TCG Live Expanded client inventory. Build validation in the Live deck editor remains required before event play.

Primary references:

- Official Pokémon Trading Card Game rules and resources: `https://www.pokemon.com/us/pokemon-tcg/rules/`
- Official Pokémon Trading Card Game card database: `https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/`
- Play! Pokémon rules, resources, and tournament documents: `https://www.pokemon.com/us/play-pokemon/about/tournaments-rules-and-resources/`
- Supplied card corpus audit: `data/card_audit.json`
- Community ruling cross-check for difficult historical interactions: Pokémon TCG Compendium, `https://compendium.pokegym.net/`

The simulator only relies on effects that matter before the first Apex Dragon attack. This document records more interactions than the simulator currently executes so future extensions have a traceable rules basis.

## Core game sequence

### Deck construction and zones

- A normal deck contains exactly 60 cards.
- Each player has a deck, hand, Active Spot, Bench, Prize cards, discard pile, and Lost Zone when relevant.
- The Bench limit is five Pokémon.
- A Basic Pokémon may enter play directly from hand. A Stage 1 or Stage 2 Pokémon needs a valid evolution route unless an effect specifically bypasses it.
- A player loses if they must draw from an empty deck, if they have no Pokémon in play after a Knock Out, or if their opponent takes all required Prize cards.

### Setup order

The correct setup sequence for this analysis is:

1. Shuffle the 60-card deck.
2. Draw seven cards.
3. If the hand contains no Basic Pokémon, reveal the mulligan condition, shuffle, and draw a new seven-card hand. The opposing player may draw a card after each opposing mulligan.
4. Choose one Basic Pokémon from the opening hand as the Active Pokémon. Each player may also place legal Basic Pokémon from that opening hand on their Bench.
5. After both players complete these opening placements, place six Prize cards from the remaining deck face down.
6. Determine turn order and begin the game.
7. At the start of each player turn, that player draws one card, including their first turn.

Therefore, the opening seven cards are used to select the Active Pokémon before Prize cards are taken. The Prize cards come from the remaining deck, not from a fresh deck before the opening hand.

### First-turn restrictions and turn actions

- The player who goes first cannot play a Supporter card on their first turn.
- The player who goes first cannot attack on their first turn.
- A player may attach one Energy card from hand each turn. Effects can attach additional Energy.
- A player may play one Supporter card each turn.
- A player may play Item cards in any number unless an effect prevents it.
- A player may play one Stadium card from hand each turn, subject to the Stadium replacement rules.
- A Pokémon cannot evolve on the player’s first turn.
- A Pokémon cannot evolve during its first turn in play. A Regidrago V placed during setup becomes eligible to evolve on that player’s turn 2. A Regidrago V Benched during turn 2 becomes eligible no earlier than turn 3.
- An attack ends the player’s turn. Effects that add a turn, such as Timeless-GX, alter the turn sequence under their own text.

### Retreat and switching

- A player may retreat an Active Pokémon by paying its Retreat Cost in Energy unless an effect changes that cost.
- A retreat does not use the manual Energy attachment for the turn.
- Switching effects replace the Active Pokémon without paying the usual Retreat Cost.
- Latias ex’s Skyliner makes each Basic Pokémon in play have no Retreat Cost while its Ability is active. The active Basic can retreat for zero, then the player promotes a chosen Benched Pokémon.

### Search, reveal, discard, and “up to”

- A card that searches for a specific class of card can only select cards meeting that printed class.
- “Up to” allows fewer than the stated maximum when the effect permits it.
- A discard cost must be paid when a card says it can only be played if a card is discarded.
- A card placed into the discard from the deck by Brilliant Blender or Professor Burnet is not a hand discard and does not consume a DCI hand resource.
- A searched card that is revealed changes hidden information. The simulator treats the remaining deck composition as known to the player for legal search selection. It does not inspect future draw order.

## Global Rule Box and VSTAR details

### Path to the Peak

Path to the Peak says Pokémon with a Rule Box in play have no Abilities. It disables Rule Box Pokémon Abilities such as:

- Tapu Lele-GX’s Wonder Tag.
- Latias ex’s Skyliner.
- Mawile-GX’s Captivating Wink.
- Regidrago VSTAR’s Legacy Star.
- Hisuian Goodra VSTAR’s Moisture Star.
- Mega Dragonite ex’s Sky Transport.

It does not remove attacks. Regidrago VSTAR may still use Apex Dragon while Path to the Peak is in play.

### Forest Seal Stone through Path to the Peak

Forest Seal Stone is a Pokémon Tool with its own VSTAR Power. Its text allows the attached Pokémon V to use the VSTAR Power printed on the Tool. The VSTAR Power is on the Tool rather than an Ability printed on the Pokémon. Path to the Peak removes Abilities from Rule Box Pokémon. It does not remove the Tool’s printed VSTAR Power. The registered model therefore permits Forest Seal Stone through Path to the Peak.

Operational limits still matter:

- The Tool must be played as an Item, so Item lock prevents attaching it.
- It needs an eligible Pokémon V in play before evolution. This list uses Regidrago V.
- The player cannot use more than one VSTAR Power in a game. Forest Seal Stone competes with Legacy Star and Moisture Star.
- Forest Seal Stone occupies the Pokémon’s one Tool slot. A player cannot also attach Powerglass until the first Tool is removed or otherwise leaves play.
- Field Blower can discard the player’s own Forest Seal Stone. This creates a legal line to attach Powerglass later.

## Card-by-card interaction register

### Pokémon

| Card | Confirmed text consequence | Setup and matchup consequence |
|---|---|---|
| Regidrago V | Basic Dragon Pokémon V. Celestial Roar discards the top three cards and attaches Energy among them. Dragon Laser costs GGF. | The simulator does not use Celestial Roar during a protected-singleton setup because blind milling can destroy valuable one-of cards. Regidrago V is the preferred opening Active and energy holder. |
| Regidrago VSTAR | Apex Dragon costs GGF and chooses an attack from a Dragon Pokémon in the discard pile. Legacy Star is a once-per-game VSTAR Power. | Apex uses Regidrago’s GGF attack cost. The copied attack’s printed energy cost is not paid again. The copied attack’s text still applies to Regidrago. |
| Dragapult ex | Stage 2 Dragon ex. Phantom Dive deals 200 and distributes six damage counters on the opponent’s Bench. | It is a valid Dragon discard payload even though it is not played as an Evolution in this list. It is S-tier for setup success. |
| Mega Dragonite ex | Stage 2 Dragon Mega Evolution ex. Ryuno Glide deals 330 and discards two Energy from itself. | Apex Dragon copies Ryuno Glide. Regidrago discards two of its own attached Energy after the attack. Powerglass may attach one Basic Energy from the discard at end of turn, leaving one manual Energy attachment to restore the third Energy on the next turn. |
| Dialga-GX, Dragon print | Basic Dragon Pokémon-GX. Timeless-GX deals 150 and grants another turn, subject to the one-GX-attack limit. | It is a legal Apex target because it is Dragon. Using Timeless-GX through Apex consumes the player’s GX attack for the game. Dialga itself is a dangerous forced-bench target because it is Basic and cannot become a useful attacker in this G/F-only list. |
| Hisuian Goodra VSTAR | Dragon VSTAR. Rolling Iron deals 200 and grants 80 damage reduction from attacks during the opponent’s next turn. | Apex applies the damage reduction to Regidrago because the copied text’s “this Pokémon” means the attacking Regidrago. It is A-tier and may be discarded in matchup-flex mode when S-tier payloads remain. |
| Tapu Lele-GX | Basic Psychic Pokémon-GX. Wonder Tag triggers when played from hand to Bench and finds a Supporter. | It cannot trigger from the opening setup or from forced benching. Path to the Peak stops Wonder Tag. It is a major route into Crispin, Steven’s Resolve, Arven, Gladion, or Burnet. |
| Latias ex, Skyliner print | Basic Psychic Pokémon ex. Skyliner gives Basic Pokémon in play no Retreat Cost. | It provides the Active-position axis. Path to the Peak stops it. It does not give free retreat to a Regidrago VSTAR itself, though it can let the existing Basic Active retreat so the VSTAR can be promoted. |
| Mawile-GX | Basic Metal Pokémon-GX. Captivating Wink triggers from hand to Bench and can force Basic Pokémon from the opponent’s hand to their Bench. | It has high discrete anti-Mimikyu value. It has low immediate setup value. Matchup-flex DCI makes it discardable when that threat is absent. Path stops Captivating Wink. |
| Oricorio GRI 55 | Basic Psychic Pokémon. Vital Dance triggers from hand to Bench and finds up to two Basic Energy cards. | It can obtain G and F without a Supporter. Path stops it. It does not trigger from opening setup or force benching. |
| Dipplin TWM 127 | Stage 1 Dragon. Syrup Catcher switches in an opposing Benched Pokémon and deals 70 to the new Active. | It cannot be opened or Benched because the list has no Applin. It is a Dragon Apex target and matchup-specific gust payload. Its Stage 1 status keeps it out of Quick Ball, Hisuian Heavy Ball, and initial Basic counts. |

### Search, discard, and acceleration cards

| Card | Confirmed text consequence | Model treatment |
|---|---|---|
| Brilliant Blender | Search the deck for up to five cards and discard them, then shuffle. It is an ACE SPEC. | Blender has no hand discard cost. It is the cleanest strict-JIT deck payload outlet. The model usually discards one A/S payload on the attack turn. Additional deck discards would be legal, though they add exposure after the attack. |
| Mysterious Treasure | Discard one card from hand. Search for a Psychic or Dragon Pokémon. | It finds Regidrago V, Regidrago VSTAR, Tapu Lele-GX, Latias ex, Oricorio, Dragon payloads, and Dipplin. It cannot find Mawile-GX. Its real usability is gated by DCI. |
| Quick Ball | Discard another card from hand. Search for a Basic Pokémon. | It finds Regidrago V, Dialga-GX, Tapu Lele-GX, Latias ex, Mawile-GX, and Oricorio. It cannot find Regidrago VSTAR, Dragapult ex, Mega Dragonite ex, Goodra VSTAR, or Dipplin. |
| Earthen Vessel | Discard another card from hand. Search for up to two Basic Energy cards. | It searches Grass and Fire. It doubles as a strict-JIT payload outlet only when the payload is already in hand and the Energy search remains useful. |
| Arven | Search the deck for an Item card and a Pokémon Tool card. | The key package is Brilliant Blender plus Forest Seal Stone. It uses the Supporter slot and therefore competes with Crispin. |
| Crispin | Search for up to two Basic Energy cards of different types. Put one into hand and attach the other to a Pokémon. | In this deck it can find one Grass and one Fire. A Regidrago with one previous Energy can reach GGF with Crispin plus the manual attachment. It cannot take two Grass. |
| Professor Burnet | Search the deck for up to two cards and discard them. | This is an unconditional deck-discard outlet. It competes with Crispin and Arven for the Supporter slot. It is especially relevant under Item lock after the Energy axis is already complete. |
| Serena | Choose discard-and-draw to five or gust an opposing Benched Pokémon V. The draw option requires at least one hand discard. | It is a fallback JIT hand-discard route when Crispin is no longer needed. Its gust choice is mutually exclusive with the draw choice. |
| Tate & Liza | Shuffle hand into deck and draw five, or switch Active with a Benched Pokémon. | The draw option discards no cards. The switch option uses the Supporter slot and competes with Crispin. It is treated as a weak setup fallback. |
| Steven’s Resolve | Search for up to three cards, put them into hand, then end the turn. | The primary selected package is Regidrago VSTAR, Crispin, Brilliant Blender. It has maximum value on a going-second turn 1 with Regidrago V and a manual Energy already established. It loses value when the opponent can disrupt the held three-card package. |
| Gladion | Look at Prize cards, put one into hand, then shuffle Gladion into the remaining Prizes. | Gladion can recover a prized Regidrago VSTAR, Blender, Crispin, key payload, or other singleton. It does not recover a second Prize in the same use. It consumes the Supporter slot. |
| Hisuian Heavy Ball | Look at Prize cards. Reveal a Basic Pokémon, put it into hand, and put Heavy Ball as a Prize. | It can recover Regidrago V, Dialga-GX, Tapu Lele-GX, Latias ex, Mawile-GX, or Oricorio. It cannot recover Regidrago VSTAR or any Stage 2 payload. |

### Recovery, disruption, and tools

| Card | Confirmed text consequence | Interaction consequence |
|---|---|---|
| Guzma | Switches an opposing Benched Pokémon with Active, then switches the player’s Active with a Benched Pokémon. | It creates gust and a switching line. Latias can later repair an unwanted Basic Active position where applicable. It has high discrete prize-race value and low early setup value. |
| Channeler | Removes all effects of attacks on the player and each of their Pokémon. | It clears self-attack effects such as “cannot attack next turn” where applicable. It does not generically turn off an opponent’s Item lock, Ability lock, Stadium, or passive effect. |
| Lusamine | Returns two Supporter and/or Stadium cards from discard to hand. | It can recover Team Yell’s Cheer, stadiums, or a needed Supporter. It cannot return Items, Tools, or Pokémon. |
| Team Yell’s Cheer | Shuffles up to three Pokémon and/or Supporter cards from discard into deck, excluding itself. | It can return Lusamine and create the recursive anti-deck-out concept, provided one member of the loop remains accessible. It cannot return Items, Tools, Stadiums, or Energy. |
| Roseanne’s Backup | Choose one or more categories: Pokémon, Tool, Stadium, Energy. Shuffle one selected card of each selected category into deck. | It returns a maximum of one card from each listed category. It cannot return Supporters. It can restore a Dragon payload, Forest Seal Stone or Powerglass, a Stadium, and one Energy in the same resolution. |
| Professor Turo’s Scenario | Return one Pokémon in play to hand and discard all cards attached to it. | It can remove a force-benched Dialga-GX. It discards that Pokémon’s attached cards, so it can be costly on an Energy-loaded Regidrago. |
| Powerglass | At the end of the player’s turn after the attack, if attached Pokémon is Active, attach a Basic Energy from discard to it. | After copied Ryuno Glide, it can recover one of the two discarded Basic Energy. It triggers after the attack and can only attach one Basic Energy. |
| Field Blower | Discard up to two Tools and/or Stadiums in play. | It may discard the player’s own Forest Seal Stone to free the Tool slot for Powerglass. It can also clear Jamming Tower, Tool Jammer, Path, or other legal targets. |
| Chaotic Swell | A Stadium played from hand causes Swell and the new Stadium to be discarded before the new Stadium has effect. | It preemptively trades with an opposing Stadium played from hand. It does not itself search or establish the energy/evolution/payload axes. |
| Path to the Peak | Rule Box Pokémon in play have no Abilities. | See the Rule Box section. The deck can use it as disruption while retaining Apex Dragon attacks and Forest Seal Stone access. |

## Apex Dragon copied-attack rules

### Attack cost and effects

Apex Dragon’s printed cost is GGF. Once Regidrago pays that cost, it chooses an attack from a Dragon Pokémon in the discard and uses that attack as its own. The selected attack’s own printed Energy cost is not separately paid. Effects belonging to the selected attack still resolve using the attacking Regidrago as “this Pokémon.”

Examples:

- **Phantom Dive:** Regidrago deals 200 and places six damage counters among the opponent’s Benched Pokémon.
- **Ryuno Glide:** Regidrago deals 330 and must discard two Energy attached to itself. It needs at least two Energy available to discard after damage resolution.
- **Rolling Iron:** Regidrago deals 200 and takes 80 less damage from attacks during the opponent’s next turn.
- **Timeless-GX:** Regidrago deals 150, takes another turn, skips the between-turns step, and uses the player’s once-per-game GX attack.
- **Syrup Catcher:** Regidrago can use the switch-in effect and then deal 70 to the new Active Pokémon.

### JIT timing

Under strict JIT, the target must be placed in discard during the same turn as the Apex attack. This prevents the opponent from receiving priority between the discard and that attack. The simulator marks every card discarded in the present turn and requires an A/S target in that local list.

After the attack, the selected payload remains exposed in discard. A single Return Label, Lysandre Prism Star, Girafarig, Surprise Box plus hand disruption, or similar effect can make the same attack unavailable or unsafe later. The simulator does not claim the payload is permanently reusable.

## Interaction corrections and caveats

1. **Brilliant Blender is not a five-card hand discard.** Its deck-discard text makes its AMR materially different from Secret Box, Ultra Ball, and Mysterious Treasure.
2. **Forest Seal Stone can function through Path to the Peak.** Item lock still blocks the Tool from being played.
3. **Channeler is self-cleanup for effects of attacks.** It is not a universal lock answer.
4. **Dipplin TWM 127 is Stage 1.** The list has no Applin, so its only current role is as a discard payload.
5. **No Water or Lightning Energy is needed for copied Ryuno Glide.** Regidrago pays Apex Dragon’s GGF cost. The copied attack still discards two attached Energy.
6. **Powerglass does not fully re-power Ryuno Glide alone.** It returns one Energy after the two-Energy discard. One manual attachment on the next turn completes the three-Energy attack requirement.
7. **Latias ex does not make a Regidrago VSTAR’s own retreat free.** It allows the current Basic Active to retreat at zero so a benched VSTAR can be promoted.
8. **The paper Expanded legality field in the supplied corpus is not a Pokémon TCG Live availability signal.** The Live deck builder is the authoritative legality check for this precise client format.
