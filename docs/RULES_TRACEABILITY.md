# Rules and Rulings Traceability Register

## Purpose

`regidrago_sim --simulate-this` produces a turn-by-turn proof trail. Every game-state change made by the simulator emits a **rule ID**. This document maps every state-changing game-mechanic implementation to the card text or core game procedure that authorizes it.

The trace is deliberately separated into two categories:

- `R-*` means a game rule, card text, or scenario lock rule.
- `P-*` means a **simulation policy choice**. It is not Pokémon TCG law. The policy is used only to choose among legal actions.

The card text was checked against the supplied `pokemon-tcg-data` corpus. Identifiers below name the exact print modeled. Direct raw URLs are recorded in `RULE_SOURCES.md` and at the top of `src/regidrago_sim.cpp`. The Expanded legality of individual cards remains an external format assumption rather than a claim made by this engine.

## Core procedure IDs

| Rule ID | Implementation sites in `src/regidrago_sim.cpp` | Rule / ruling represented |
|---|---|---|
| `R-GAME-SETUP` | `setup`, `choose_opening_active`, `choose_opening_bench` | Start from a shuffled 60-card deck, draw 7, mulligan until a Basic Pokémon is present, choose a Basic Active, optionally Bench Basics, then set 6 Prize cards. Tapu Lele-GX and Oricorio are retained in hand at setup because their Abilities require being played from hand to Bench **during a turn**. |
| `R-GAME-DRAW` | `begin_turn`, `draw_card`, draw effects | The player draws at the start of each turn except the player who goes first does not draw on that player’s first turn. Every modeled search and draw stops if deck exhaustion occurs. |
| `R-GAME-DECK-OUT` | `begin_turn`, `run` | A player with no cards remaining in deck at the beginning of that player’s turn loses before turn actions. https://www.pokemon.com/us/pokemon-tcg/rules |
| `R-GAME-FIRST-TURN` | `first_turn_going_first`, `supporter_allowed`, `begin_turn`, `use_celestial_roar` | The player going first cannot play a Supporter or attack on that player’s first turn. Item cards, Basics, Tools, manual attachments, and non-attack effects remain separately governed. |
| `R-GAME-BENCH` | `bench_from_hand`, `play_basics_from_hand` | Only Basic Pokémon may be played from hand to the Bench. The Bench limit is five. |
| `R-GAME-ENERGY` | `attach_manual`, `play_crispin`, `use_celestial_roar` | At most one Energy card may be manually attached per turn. Effects may attach additional Energy. The policy chooses an Energy type only when it advances the model’s GGF target. |
| `R-GAME-SUPPORTER` | `supporter_allowed`, all `play_*` Supporter methods | At most one Supporter may be played per turn, before attacking. The going-first first-turn restriction is enforced centrally. |
| `R-GAME-ITEM` | all Item methods | Item cards may be played during the turn, subject to their own conditions and a modeled Item-lock scenario. |
| `R-GAME-TOOL` | `attach_fss`, `Pokemon::tool` | A Pokémon Tool can be attached only to a Pokémon without a Tool. Forest Seal Stone is played from hand as an Item/Tool and therefore cannot be attached during a modeled Item lock. |
| `R-GAME-EVOLVE` | `evolve_best_regi` | A player may not evolve on the player’s first turn, nor evolve a Pokémon on the same turn it entered play. The simulator checks `entered_turn < current_turn`. |
| `R-GAME-RETREAT` | `retreat_to_benched_vstar_with_latias`, `retreat_used` | Retreat is limited to once per turn. Latias ex can make a Basic Active’s Retreat Cost zero, allowing that legal retreat without Energy payment. |
| `R-VSTAR-01` | `vstar_power_used`, `use_fss`, `use_legacy_star` | Only one VSTAR Power may be used in a game. Forest Seal Stone Star Alchemy and Regidrago VSTAR Legacy Star share this single-use state. |

## Card and interaction IDs

| Rule ID | Exact print / text modeled | Implementation sites | Important ruling treatment |
|---|---|---|---|
| `R-RV-01` | Regidrago V `swsh12-135`, **Celestial Roar**: one Colorless cost; discard the top 3; attach any Energy among them to itself. | `use_celestial_roar` | Requires at least one attached Energy to attack. It is only attempted on turn 1 while going second, after all legal setup actions and only if the turn was not ended by Steven’s Resolve. Attached Energy leaves the temporary discard path; only non-Energy cards remain in discard. |
| `R-RVS-01` | Regidrago VSTAR `swsh12-136`, **Apex Dragon** costs GGF and uses an attack from a Dragon Pokémon in discard; **Legacy Star** discards top 7 and returns up to 2 cards. | `record_ready`, `use_legacy_star` | Readiness requires Active VSTAR with at least GGF plus a permitted Dragon payload. Legacy Star shares the VSTAR-Power limit and is suppressed by the modeled Rule Box Ability lock. |
| `R-FSS-01` | Forest Seal Stone `swsh12-156` is an Item and Pokémon Tool. | `attach_fss` | It may be attached only to Regidrago V in this model, while that Pokémon has no Tool. It cannot be played from hand during Item lock. |
| `R-FSS-02` | Forest Seal Stone `swsh12-156`, **Star Alchemy** searches for any card; its card text gives the attached Pokémon V the VSTAR Power. | `use_fss`, `fss_target_after_search_started` | Uses the shared VSTAR-Power state. The simulator treats activation of already-attached FSS as distinct from playing an Item from hand. It refuses use after the holder has evolved because the model treats the holder as no longer a Pokémon V for FSS’s grant condition. |
| `R-MT-01` | Mysterious Treasure `sm6-113`: discard a card; search a Psychic or Dragon Pokémon. | `play_mysterious_treasure` | A discard cost must be legal before play. Target selection is inspected only after the search begins, with a legal fallback target. |
| `R-QB-01` | Quick Ball `swsh1-179`: discard another card; search a Basic Pokémon. | `play_quick_ball` | The searched Basic is chosen only after the search begins. A Quick Ball cost cannot be itself. |
| `R-UB-01` | Ultra Ball comparator: discard 2 other cards; search a Pokémon. | `play_ultra_ball` | Variant-only comparator. The two costs are selected as two distinct cards. It is not in the submitted baseline 60-card list. |
| `R-EVOINCENSE-01` | Evolution Incense comparator: search an Evolution Pokémon. | `play_evolution_incense` | Variant-only comparator. The preferred target is Regidrago VSTAR; another legal Evolution may be found if VSTAR is absent. |
| `R-EV-01` | Earthen Vessel `sv4-163`: discard another card; search up to 2 Basic Energy. | `play_earthen_vessel`, `choose_energy_targets_after_search_started` | Two copies of the same Basic Energy are legal. The selector can choose G+F, G+G, or F+F according to legal remaining targets. |
| `R-ORICORIO-01` | Oricorio `sm2-55`, **Vital Dance**: on play from hand to Bench during the turn, search up to 2 Basic Energy. | `bench_oricorio_if_useful`, `resolve_entry_ability` | Oricorio is not a Rule Box Pokémon. A Path-style Rule Box Ability lock does not stop Vital Dance. |
| `R-TAPU-01` | Tapu Lele-GX `cel25c-60_A`, **Wonder Tag**: on play from hand to Bench during the turn, search a Supporter. | `bench_tapu_if_useful`, `resolve_entry_ability`, `choose_supporter_after_search_started` | The Ability is suppressed by the modeled Rule Box Ability lock. The policy benches it only when a Supporter connector solves a currently missing axis. |
| `R-LATIAS-01` | Latias ex `sv8-76`, **Skyliner**: Basic Pokémon in play have no Retreat Cost. | `can_free_retreat_with_latias`, `retreat_to_benched_vstar_with_latias` | It affects the Basic Active, not a VSTAR. The model uses it to retreat a Basic Active and promote a Benched VSTAR. It is suppressed by a Rule Box Ability lock. |
| `R-BLENDER-01` | Brilliant Blender `sv8-164`: search up to 5 cards and discard them. | `play_brilliant_blender` | It searches the deck and discards selected cards. It does not discard from hand. The model chooses a payload and, where present, dead Dipplin as legal DCI discard. |
| `R-CRISPIN-01` | Crispin `sv7-133`: search up to 2 **different-type** Basic Energy; put one in hand and attach the other. | `play_crispin` | A Grass plus Fire pair is required for the attachment branch. If only one type is legally found, the engine puts it in hand and attaches none. |
| `R-BURNET-01` | Professor Burnet `swsh12tg-TG26`: search deck for up to 2 cards and discard them. | `play_professor_burnet` | The engine selects a Dragon payload and may add dead Dipplin. It is the explicit Item-lock payload bridge. |
| `R-SERENA-01` | Serena `swsh12-164`: choose the discard/draw mode, discard **up to 3** cards, then draw to five. | `play_serena` | Zero to three discards is legal by card text. The simulator intentionally requires at least one policy-safe discard before spending Serena, then may discard up to two additional policy-safe cards in every DCI profile. |
| `R-TATE-01` | Tate & Liza `sm7-148`: choose shuffle-and-draw-five or switch Active with Benched. | `play_tate_draw`, `play_tate_switch` | The simulator selects switch when it immediately completes the Active VSTAR axis; otherwise the draw mode requires a sufficiently low hand. |
| `R-STEVEN-01` | Steven’s Resolve `sm7-145`: search up to 3 cards; turn ends. | `should_play_steven`, `play_steven` | It is used only going second on turn 1 with at least two missing setup axes. In turn-two Item-lock scenarios it fetches Burnet rather than Blender. |
| `R-ARVEN-01` | Arven `sv1-166`: search an Item and a Pokémon Tool. | `play_arven` | The policy avoids Arven under an active Item lock because the Item line cannot be used then. Searches have legal fallback behavior. |
| `R-GLADION-01` | Gladion `sm4-95`: look at face-down Prizes, take one, then shuffle Gladion into remaining Prizes. | `play_gladion` | The decision to use Gladion does not inspect Prize identities. Once used, exactly one Prize is exchanged even if the desired card is not prized. Gladion does not enter discard. |
| `R-HEAVYBALL-01` | Hisuian Heavy Ball `swsh10-146`: inspect face-down Prize cards; reveal a Basic Pokémon, put it in hand, and replace it with Heavy Ball. If no Basic is revealed, discard Heavy Ball. | `play_heavy_ball` | At K0 the policy plays Heavy Ball for Prize information before ordinary search Items. It prioritizes a missing Regidrago V, then a live Tapu Lele-GX Supporter chain, except that Oricorio takes priority when it preserves a live in-hand Burnet payload bridge under Item lock. At K1 it is held only when no Prize Basic exists. |
| `R-PATH-01` | Path to the Peak `swsh6-148`: Pokémon with Rule Boxes have no Abilities. | `rule_box_abilities_locked`, `ability_available_for_pokemon` | Scenario abstraction: suppresses Tapu Lele-GX, Latias ex, and Legacy Star; does not suppress Oricorio. Existing Forest Seal Stone activation is explicitly documented separately by `R-FSS-02`. |

## Policy IDs

| Policy ID | Implementation sites | Meaning |
|---|---|---|
| `P-HIDDEN-01` | `might_be_unseen`, action gates | Before an effect legally reveals/searches a hidden zone, decisions use only public/known zones and deck copy counts. Debug Prize output is never read by policy. |
| `P-AXIS-01` | `run_turn`, target priority helpers | The engine first identifies missing axes: Regidrago V, VSTAR, Energy, Active position, and legal payload timing. It avoids consuming a connector unless it advances one of them. |
| `P-CONNECTOR-01` | `needs_oricorio_connector`, `needs_tapu_connector`, search item ordering | A weaker connector is held when a present legal connector already covers the same axis. Example: do not Bench Oricorio merely to get Energy while productive Crispin or Earthen Vessel already solves the turn. |
| `P-COMPRESS-01` | `play_heavy_ball` | Under Item lock, when a manual attachment can finish the only missing Energy and Professor Burnet in hand is the live payload bridge, prize Oricorio is selected over prize Tapu Lele-GX so Burnet retains the Supporter slot. |
| `P-DCI-01` | `choose_discard`, Blender/Burnet/Serena selection | Dead Dipplin is disposable because the 60-card list contains no Applin. Matchup-flex additionally permits a documented set of matchup cards as discard fodder. |
| `P-JIT-01` | `strict_payload_timing`, `payload_ready`, `can_play_payload_this_turn` | Strict and matchup-flex profiles require a payload to enter discard in the turn that produces the ready state. No-discard-control permits earlier banking. |

## Trace contract

A trace line has this form:

```text
T2 | PLAY SUPPORTER | rules: R-CRISPIN-01; R-GAME-SUPPORTER | ...
```

A line must name the rule IDs for every game mechanic it changes. `simulate-this` prints the player-known opening state, a debug-only Prize list, every draw, discard, search, attachment, evolution, retreat, VSTAR Power, attack, and ready-state check. The trace is the review surface for legal sequencing. It is not an opponent model.
