# Rules Reverification Pass

## Method

This pass rechecked the modeled card text against the supplied `pokemon-tcg-data` corpus and matched the implementation to the traceability register. The fixture suite tests the decisions that matter to the setup objective. It does not claim that the project is a complete Pokémon TCG rules engine.

## Card-text coverage rechecked

| Mechanic | Exact modeled print | Verified behavior |
|---|---|---|
| Celestial Roar | Regidrago V `swsh12-135` | Requires one Colorless Energy, discards top three, attaches every Energy among them to itself. |
| Apex Dragon and Legacy Star | Regidrago VSTAR `swsh12-136` | GGF ready predicate; Legacy Star uses the shared VSTAR Power and current-turn discard bookkeeping. |
| Vital Dance | Oricorio `sm2-55` | Triggers only when played from hand to Bench during a turn and is not disabled by a Rule Box lock. |
| Wonder Tag | Tapu Lele-GX `cel25c-60_A` | Triggers only on play from hand to Bench and is disabled by a Rule Box lock. |
| Skyliner | Latias ex `sv8-76` | Applies only to a Basic Active for retreat purposes. |
| Brilliant Blender | `sv8-164` | Searches deck and discards selected cards. It never discards from hand. |
| Mysterious Treasure / Quick Ball | `sm6-113` / `swsh1-179` | Pay a real hand-discard cost, then search an eligible target with a legal fallback after deck inspection. |
| Earthen Vessel | `sv4-163` | Can find up to two Basic Energy, including two of the same type. |
| Crispin | `sv7-133` | Needs a two-type Energy selection to attach one and put one in hand. A one-type resolution attaches none. |
| Professor Burnet | `swsh12tg-TG26` | Searches and discards payloads, including the Item-lock bridge line. |
| Serena | `swsh12-164` | Draw mode requires one to three discards; it is held after ready state. |
| Tate & Liza | `sm7-148` | Models both draw and switch modes. |
| Steven's Resolve | `sm7-145` | Searches up to three and ends the turn; planned around next-turn Item lock. |
| Arven | `sv1-166` | Is held during active Item lock. |
| Gladion / Heavy Ball | `sm4-95` / `swsh10-146` | Do not inspect Prize identities before the effect; resolve the required exchange after the legal reveal. |
| Forest Seal Stone | `swsh12-156` | Attaches only to the modeled Pokémon V holder, shares the VSTAR Power state, and remains an existing Tool rather than an Item-from-hand play after attachment. |

## Explicit policy checks

The new fixture suite checks that the engine does not spend a Supporter or connector unless it advances an unfilled axis: Regidrago V, VSTAR, GGF, current-turn payload timing, or Active positioning. It also checks JIT profile separation, fallback searches, item-lock planning, and holding draw cards after the objective is complete.

## Scope exclusions

The project still does not model opponent-side damage, Knock Outs, Prize taking, full Stadium sequencing, hand disruption, gust, Mimikyu Copycat, Return Label, Lysandre Prism Star, or full Expanded legality. Those remain out of scope and should not be inferred from a passing setup-policy fixture.
