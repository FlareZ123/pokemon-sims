# Policy Decision Register

## Scope

This simulator is a single-player, imperfect-information setup model. Its objective is the earliest legal ready state:

- Active Regidrago VSTAR;
- at least `GGF` attached to the selected Regidrago line; and
- a permitted Dragon payload in discard under the selected DCI/JIT profile.

It does not model opponent turns, damage, prizes taken, or a global two-player game-tree solution. "Optimal" in this document means the best legal action among the engine's modeled connector routes and visible state.

### A/S payload classification

`is_payload` is the single readiness and payload-routing predicate. It contains Dragapult ex, Mega Dragonite ex, Dialga-GX, Hisuian Goodra VSTAR, and Appletun: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/swsh12-136. Dipplin TWM 127 remains a legal Dragon target for Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/sv6-127.

Maintainer policy excludes Dipplin's Syrup Catcher from the A/S win-condition set. The attack can create a gust-like target, yet it may take no Prize and allows an opponent to switch after the effect. That line is insufficient for this deck's modeled ready-state definition. Dipplin therefore never completes the payload axis, never makes `payload_ready()` true, and never becomes a payload-discard target.

## Knowledge states

### K0: before a legal inspection

At K0 the policy may use only public game state, hand, board, discard, known card-copy counts, and scenario flags. It does not read Prize identities or the remaining deck order.

Examples:

- Gladion is used blindly only when every ordinary route to a critical axis is absent.
- **Heavy Ball is played at K0 whenever Items are legal.** It is an information action: it reveals Prize cards, may recover a Basic Pokémon, and establishes K1. It is not held merely because Quick Ball, Mysterious Treasure, Ultra Ball, or Arven is also present.
- A discard cost is selected from observable hand state, never because the engine knows an Energy is safe in Prizes.

### K1: after a legal deck or Prize inspection

A resolved deck search physically exposes the deck. A resolved Heavy Ball or Gladion physically exposes the full Prize set. In either case, the fixed 60-card recipe plus known zones lets the policy deduce the complementary hidden zone. The engine records this as `DECK KNOWLEDGE` or Prize-reveal knowledge and can then use Prize-aware decisions.

Examples:

- Wonder Tag or Forest Seal Stone chooses Gladion when inspection proves the missing Regidrago VSTAR is prized.
- Heavy Ball is held only when K1 proves that no Basic Pokémon remains in the Prize cards; in that state it is retained as a legal discard cost.
- If Heavy Ball recovers Tapu Lele-GX, the next stabilization pass Benches it and resolves Wonder Tag when a Supporter connector is useful.
- A revealed Prize Fire makes Crispin dead when the fixed-list count shows no Fire can remain in deck. The policy then stops reserving FSS, Oricorio, or a Supporter slot for that Crispin line.
- Gladion can retrieve a final prized Grass or Fire only when that one manual attachment completes all other ready axes. It does not preempt a still-missing payload bridge such as Burnet under Item lock.

K1 begins only during a legal effect resolution. It does not grant prior knowledge retroactively.

## Decision priorities

1. **Immediate exact missing axis.** A known prized Regidrago V or VSTAR retrieved through Gladion outranks slower connector Supporters when it is the missing card axis.
2. **Current-turn direct completion.** Crispin outranks Arven, Oricorio, Earthen Vessel, and Ultra Ball chains when it can actually complete the remaining GGF component immediately. A Crispin known unable to find a required type is not a live line.
3. **Turn-one compression.** Going second on turn one, Steven's Resolve is selected when it covers at least two unresolved axes. Under a scheduled turn-two Item lock it takes Professor Burnet rather than Brilliant Blender.
4. **Heavy Ball information and connector ordering.** At K0, Heavy Ball is played before ordinary search Items. It takes the best revealed Basic for the live state: missing Regidrago V, then a live Tapu Lele-GX Supporter chain, Oricorio Energy compression, Latias active-position recovery, then prized Dialga-GX when payload is the only missing axis and a legal same-turn one-discard Item survives, then the best remaining future connector or discard-fodder Basic: https://api.pokemontcg.io/v2/cards/swsh10-146 https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12-136
5. **Shortest search chain.** Forest Seal Stone targets Oricorio over a single Energy and Earthen Vessel over Arven only when no live direct Energy Supporter already solves the turn. It takes a direct Supporter over a Tapu Lele-GX chain when both cover the same immediate axis.
6. **Preserve resources when another card already solves the axis.** The engine does not Bench Tapu Lele-GX merely to find Arven if VSTAR is already in hand. It preserves Forest Seal Stone when Tate & Liza alone fixes Active position or productive Crispin alone fixes Energy. It also holds Ultra Ball rather than discarding two cards for Oricorio when Crispin is live.
7. **Legal post-search fallback.** Arven, Wonder Tag, and other search effects choose an available same-axis fallback after the deck is inspected. Mysterious Treasure includes Dipplin as a legal Dragon fallback when preferred targets are unavailable. Its separate A/S policy classification still keeps it outside readiness and payload-routing decisions.
8. **Draw Supporter refresh.** Tate & Liza draw mode has no printed hand-size cap. The policy uses it only after direct connectors fail, as a legal shuffle-and-draw-five refresh through the one-Supporter-per-turn limit: https://api.pokemontcg.io/v2/cards/sm7-148 https://www.pokemon.com/us/pokemon-tcg/rules
9. **Zone integrity.** Celestial Roar Energy is attached to Regidrago V and removed from its temporary discard path; only non-Energy cards remain in discard after the attack resolves.

## Scenario-lock treatment

- Item lock prevents Items from being played from hand. It does not retroactively remove an already attached Forest Seal Stone from play.
- Rule Box Ability lock suppresses Tapu Lele-GX Wonder Tag, Latias ex Skyliner, and Regidrago VSTAR Legacy Star. It does not suppress Oricorio Vital Dance because Oricorio has no Rule Box.
- Under a Rule Box Ability lock, the engine does not Bench or search for Tapu Lele-GX or Latias ex as an Ability connector. If a benched VSTAR must become Active, it can search for or use Tate & Liza's switch mode instead.

## DCI/JIT treatment

- Strict JIT and matchup-flex JIT require the payload to enter discard during the same player turn that creates readiness.
- No-discard-control permits earlier payload banking.
- The strict profile protects the only payload from an early discard. Matchup-flex can use the documented matchup-only fodder list. Dead Dipplin is always a legal modeled discard because the submitted list contains no Applin.

## Test surfaces

- `regidrago_unified_tests regidrago_policy_tests`: 57 executable core exact-state rule and policy fixtures. Canonical inventory: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191
- `regidrago_unified_tests regidrago_tier2_tests`: 31 executable choice-differentiation, fast-compression, K1, and lock-aware fixtures. Canonical inventory: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73
- `--simulate-this`: seeded full-game traces with state changes labeled by rule and policy IDs.

Every policy test is deterministic and starts from an explicit state. The tests constrain the model's choices; they do not constitute a proof of globally optimal play across every hidden deck order or opponent response.

## Secret Box and Pineco route policy

The `regidrago-pineco` policy compares a direct Regidrago completion against the Forretress route before spending resources. It does not assume visible Pineco pieces make the combo optimal.

A planned sequence may use Arven, Gladion, or Steven's Resolve on T1 and Dawn on T2. These are sequential Supporters, so they are not treated as same-turn contention. Secret Box pays three other cards first and reserves any additional discard needed by the selected Item. Search categories are adaptive: held Fire can remove the Forest Seal Stone requirement, prior-turn Pineco can remove Forest of Vitality, held VSTAR can change the Item target, and an existing payload can remove Dawn's Stage 2 target.

Route-aware DCI may spend a normally protected card only when the complete observable route replaces its role. This includes redundant Energy, duplicate VSTAR cards, a search Item replaced by Secret Box, or a Supporter that cannot be played after Dawn consumes the Supporter action. The strict fixture still blocks Secret Box when the third apparent cost is a unique, unreplaced VSTAR axis.

The policy models proactive Active-position recovery. Tapu Lele-GX can receive a T1 Grass attachment and retreat to an evolution-eligible Regidrago V. Exploding Energy can fund a later retreat when that creates the earliest complete route. Forest Seal Stone and Legacy Star remain mutually exclusive VSTAR Powers.

All route decisions and failure counters are recipe-gated. They do not change the shell action selector.
