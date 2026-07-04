# Card Audit and List Validation

## Result

The supplied list contains exactly 60 cards:

| Group | Count |
|---|---:|
| Pokémon | 19 |
| Trainers | 32 |
| Basic Energy | 9 |
| **Total** | **60** |

The reproducible raw audit is `data/card_audit.json`. It was generated from the user-supplied `pokemon-tcg-data-master(26).zip` archive with `scripts/audit_card_data.py`.

## Exact print identities used by the model

| List entry | Corpus card ID | Copies | Key verified property |
|---|---|---:|---|
| Regidrago V | `swsh12-135` | 4 | Basic Dragon V, Celestial Roar, Dragon Laser |
| Regidrago VSTAR | `swsh12-136` | 3 | Dragon VSTAR, Apex Dragon costs GGF |
| Dragapult ex | `sv6-130` | 2 | Stage 2 Dragon, Phantom Dive |
| Mega Dragonite ex | `me2pt5-152` | 2 | Stage 2 Dragon Mega ex, Ryuno Glide 330 |
| Dialga-GX | `sm5-100` | 1 | Dragon Basic GX, Timeless-GX |
| Hisuian Goodra VSTAR | `swsh11-136` | 1 | Dragon VSTAR, Rolling Iron |
| Tapu Lele-GX | `cel25c-60_A` | 2 | Basic Psychic GX, Wonder Tag |
| Latias ex | `sv8-76` | 1 | Basic Psychic ex, Skyliner |
| Mawile-GX | `sm11-141` | 1 | Basic Metal GX, Captivating Wink |
| Oricorio GRI 55 | `sm2-55` | 1 | Basic Psychic, Vital Dance |
| Dipplin TWM 127 | `sv6-127` | 1 | Stage 1 Dragon, Syrup Catcher |
| Brilliant Blender | `sv8-164` | 1 | ACE SPEC, deck-discard up to five |
| Mysterious Treasure | `sm6-113` | 4 | hand discard, Psychic or Dragon search |
| Quick Ball | `swsh1-179` | 3 | hand discard, Basic search |
| Earthen Vessel | `sv4-163` | 2 | hand discard, up to two Basic Energy |
| Arven | `sv1-166` | 2 | Item plus Pokémon Tool search |
| Crispin | `sv7-133` | 2 | distinct-type Basic Energy split |
| Professor Burnet | `swsh12tg-TG26` | 1 | deck-discard up to two |
| Serena | `swsh12-164` | 1 | discard/draw or Pokémon V gust |
| Tate & Liza | `sm7-148` | 1 | draw five or switch |
| Steven’s Resolve | `sm7-145` | 1 | exact three cards, end turn |
| Guzma | `sm3-115` | 1 | gust plus self-switch |
| Channeler | `sm11-190` | 1 | remove effects of attacks on own Pokémon |
| Gladion | `sm4-95` | 2 | Prize inspection and one recovery |
| Lusamine | `sm4-96` | 1 | Supporter/Stadium recovery |
| Team Yell’s Cheer | `swsh9-149` | 1 | Pokémon/Supporter deck recovery |
| Roseanne’s Backup | `swsh9-148` | 1 | selected category recovery |
| Professor Turo’s Scenario | `sv4-171` | 1 | return a Pokémon in play to hand |
| Forest Seal Stone | `swsh12-156` | 1 | Tool VSTAR Power, any card search |
| Powerglass | `sv6pt5-63` | 1 | after-attack Basic Energy recovery |
| Field Blower | `sm2-125` | 1 | discard up to two Tools/Stadiums |
| Chaotic Swell | `sm12-187` | 1 | counter-Stadium replacement |
| Path to the Peak | `swsh6-148` | 1 | Rule Box Ability suppression |
| Hisuian Heavy Ball | `swsh10-146` | 1 | Basic Pokémon Prize recovery |
| Grass Energy | `energy-grass` | 6 | basic G source |
| Fire Energy | `energy-fire` | 3 | basic R source |

## Audit outcomes that materially changed the simulation

### Brilliant Blender

The supplied corpus has this effect: search the deck for up to five cards and discard them. It does not discard cards from hand. The tool therefore implements Blender as a deck-selected JIT payload route with no hand-DCI payment.

### Mysterious Treasure

The effect discards one hand card, then searches for a **Psychic or Dragon** Pokémon. This makes it an adaptive Regidrago V, Regidrago VSTAR, Tapu Lele-GX, Latias ex, Oricorio, payload, and Dipplin connector. It cannot retrieve Mawile-GX.

### Crispin

Crispin takes up to two Basic Energy cards of **different types**, attaches one, and puts the other in hand. The simulator verifies this as one Grass plus one Fire. It never allows an illegal double-Grass Crispin search.

### Dipplin TWM 127

This print is Dragon and Stage 1. It is useful to Apex Dragon, though it is not a Basic. The supplied deck includes no Applin. The simulator does not make the illegal move of opening or benching Dipplin.

### Latias ex

The model uses `sv8-76`, the 210 HP Psychic ex whose Skyliner makes Basic Pokémon in play have no Retreat Cost. Earlier Latias ex prints have different effects and cannot be substituted in the model.

### Dialga-GX

The target needs the Dragon print. The audit uses `sm5-100`; metal-type Dialga-GX prints are excluded from Apex Dragon’s Dragon-only selection condition.

## Paper legality versus Pokémon TCG Live availability

The supplied database stores `legalities.expanded` for paper Expanded. That field has no model of Pokémon TCG Live backport status. The deck list describes these cards as the actual Live list, so the model accepts it as its game-input roster. This distinction matters when sharing results:

- The audit validates card text, type, stage, and rules text.
- The audit confirms the requested count totals.
- The audit cannot prove that a particular historic print is currently usable in the Live client.
- Live-client deck validation remains mandatory before relying on a recommendation.

## Reproduction command

```text
python scripts/audit_card_data.py C:\path\to\pokemon-tcg-data-master.zip --out data\card_audit.json
```

The script accepts either the ZIP file or an extracted repository directory. It locks its destination and replaces the JSON atomically.
