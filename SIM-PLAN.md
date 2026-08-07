# SIM-PLAN: Regidrago VSTAR Live Expanded Setup Solver

## Purpose

Estimate the probability that the supplied 60-card list reaches an attack-ready Regidrago VSTAR with a currently usable A-tier or S-tier Apex Dragon target. The simulator is a single-player, opponent-free setup model. It intentionally reports policy performance rather than an impossible claim of exact optimal play across hidden draws and unknown opponent decisions.

The analysis is built around the user-supplied concepts:

- **DCI**, a state-dependent choice of cards that may legally and strategically pay a discard cost.
- **AMR**, whether a listed connector is usable in the current hand, board, turn, and matchup state.
- **Connector domination**, where an available connector is evaluated against the stronger use of the same constrained resource.
- **JIT discarding**, which changes a payload from a generic resource into a turn-local requirement.
- **Prizing collapse**, where a visible recovery line may still lose because its bridge cards are also prized.

## Revision history

### Revision 0, model contract

1. Use the exact user-supplied 60-card list as the baseline.
2. Draw an opening hand, resolve mulligans, select an Active Basic, bench legal opening Basics, and place six Prizes after opening placements.
3. Draw one card at the beginning of every simulated turn.
4. Model player turns 1 through 5, separately for going first and going second. T4 remains the setup-success deadline; unresolved trials continue through diagnostic T5, and first readiness on T5 remains a setup failure: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md https://github.com/FlareZ123/pokemon-sims/issues/1491
5. Treat a setup as successful only when the Active is Regidrago VSTAR, it carries two Grass and one Fire Energy, and an A/S Dragon target is legally available in the discard pile.
6. Separate strict JIT, matchup-flex JIT, and no-discard-control profiles.
7. Add item-lock, Rule Box ability-lock, and combined-lock stress cases.

### Revision 1, card-text audit changes

The corpus audit changed the implementation details in several important ways.

1. Brilliant Blender searches the **deck** for up to five cards and discards them. It does not require hand cards. A strict JIT model therefore reserves Blender until an attack turn, then generally discards one immediate payload rather than treating Blender as a five-card hand-discard outlet.
2. Mysterious Treasure discards one card **from hand** and searches only for a Psychic or Dragon Pokémon.
3. Crispin searches for up to two Basic Energy cards of different types, places one in hand, and attaches the other. The modeled accelerator is therefore one Grass plus one Fire, with the manual attachment satisfying the remaining Apex requirement.
4. Forest Seal Stone is used while attached to Regidrago V. The model consumes the game-wide VSTAR Power resource and does not allow a later Legacy Star in the same simulation.
5. Latias ex’s Skyliner gives Basic Pokémon in play no Retreat Cost. The simulator uses it only to move an already-benched Regidrago VSTAR Active through a legal zero-cost retreat from the current Basic Active.
6. Dipplin TWM 127 is a Stage 1 Dragon. It cannot be an opening Basic or bench target in this list. It remains a legal Dragon search target and a discardable dead Stage 1 tech card. It is excluded from the A/S payload set.

### Revision 2, rules checkpoint

An early build mistakenly allowed a setup Regidrago V to evolve on turn 1 because its `entered_turn` value was zero. The rule engine now has both gates:

- no evolution on the player’s first turn;
- no evolution during the Pokémon’s first turn in play.

The same checkpoint added output locking and atomic replacement for result files. It also moved Forest Seal Stone’s turn-one going-second target toward Steven’s Resolve where three separate future axes are missing.

### Revision 3, connector-sequence and artifact checkpoint

The policy now recognizes two-step payload lines that are legal without a draw-order oracle:

- Mysterious Treasure may discard an in-hand JIT payload while fetching another legal Psychic or Dragon card, even when its ordinary search target is already present.
- Mysterious Treasure may fetch a deck payload when a second discard outlet remains in hand.
- Quick Ball may discard an in-hand JIT payload while taking a legal Basic target, and it may fetch Dialga-GX when a second outlet remains.
- Ultra Ball is evaluated using its actual two-card hand payment, with the dynamic DCI selector checking both discards.
- Evolution Incense is included as a deliberately narrow evolution-axis comparator.

The executable registers `regidrago-shell` and `regidrago-pineco`. Aggregate `--all-decks` runs the same scenario matrix for both recipes and emits one row per recipe and scenario: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#registered-decks https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv https://github.com/FlareZ123/pokemon-sims/issues/1493. The separate modeling-only Crobat V registry is exposed through `--model-crobat` and `--model-variant`; its temporary shell derivatives remain outside `deck_registry()`, `--all-decks`, and the canonical shell baseline, and write `results/crobat_variant_model.csv` plus `docs/CROBAT_MODEL_REPORT.md`: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc#L250-L330 https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1496. Historical generic variant-builder work remains design context; the retired generic `variant_results.csv` must not support current claims: https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7.

## Simulation state

Each trial contains the following zones and state fields.

| Area | Tracked state |
|---|---|
| Deck | Exact remaining card identities, shuffled only when an effect requires it. The policy sees composition and legal search targets. It does not inspect future draw order. |
| Hand | Exact card identities and current discard candidates. |
| Prizes | Six exact face-down identities. Gladion and Hisuian Heavy Ball inspect the modeled Prize cards. |
| Discard | Exact cards plus a `discarded_this_turn` sublist used by strict JIT. |
| Active | Pokémon identity, entry turn, Grass count, Fire count, Forest Seal Stone attachment, Powerglass attachment. |
| Bench | Up to five Pokémon with the same tracked fields. |
| Turn flags | Supporter used, manual Energy attached, VSTAR Power used, turn ended, going-first restriction, lock state. |
| Outcome counters | First ready turn, mulligans, opening Regidrago V, Forest Seal Stone use, Steven’s Resolve use, Brilliant Blender use. |

The state intentionally excludes opponent hand, damage, knockouts, prize-taking, disruption, and attack choices after the first successful Apex setup. Those variables require opponent archetype policies and would turn this project into a two-player game tree.

## Rules engine coverage

### Setup and timing

1. Start from the full 60 cards.
2. Shuffle and draw seven cards.
3. Mulligan until at least one Basic is present.
4. Select a legal Active Basic with a Regidrago V-first priority.
5. Bench selected legal opening Basics up to the five-slot limit. Opening placements do not trigger “when you play from your hand onto your Bench” Abilities.
6. Place six Prize cards from the remaining shuffled deck after opening placements; do not shuffle again: https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
7. On each own turn, draw one card.
8. Going first blocks Supporters and attacks on turn 1.
9. A manual Energy attachment is limited to one each turn.
10. A Supporter is limited to one each turn.
11. Regidrago VSTAR may evolve only after both evolution gates are satisfied.

### Connector graph

The engine encodes the high-value edges below, together with their costs and contentions.

```text
Mysterious Treasure --[discard 1 hand card]--> Regidrago V / VSTAR / Tapu / Latias / payload
Quick Ball --[discard 1 hand card]--> Regidrago V / Tapu / Latias / other Basic
Earthen Vessel --[discard 1 hand card]--> Grass + Fire in hand
Arven --> Item + Tool
Arven --> Brilliant Blender + Forest Seal Stone
Forest Seal Stone on Regidrago V --[shared VSTAR Power]--> any card
Tapu Lele-GX from hand to Bench --> selected Supporter
Oricorio GRI 55 from hand to Bench --> up to 2 Basic Energy
Crispin --> one basic energy attached + one differently typed basic energy in hand
Steven's Resolve --> VSTAR + Crispin + Blender, then turn ends
Brilliant Blender --> one JIT A/S payload from deck to discard
Professor Burnet --> one or two targets from deck to discard
Latias ex --> retreat the current Basic Active for zero to promote a benched Regidrago
```

### Explicit resource contentions

- Arven consumes the Supporter slot, so its Item and Tool package competes with Crispin, Steven’s Resolve, Burnet, Gladion, Serena, and Tate & Liza.
- Forest Seal Stone consumes the one VSTAR Power per game. It competes with Regidrago VSTAR’s Legacy Star and any other VSTAR Power.
- Mysterious Treasure, Quick Ball, and Earthen Vessel require a hand discard. Their availability depends on the active DCI profile.
- Blender provides direct deck discard. It has higher JIT AMR than a hand-discard outlet when the payload remains in deck.
- Latias ex only resolves the Active-position axis when its Ability is available and a Basic is currently Active.
- Bench space can stop Tapu, Oricorio, Latias, and backup Regidrago lines. The current early-game policy avoids benching Mawile and Dialga outside a direct reason.

## DCI policies

### Strict JIT

A/S payloads are protected until the same turn they will support Apex Dragon. Singletons with opponent-facing or recovery value remain protected. A discard cost may use a duplicate attacker/evolution, surplus Energy, or an immediate current-turn payload.

This is the conservative discard-control profile. It captures the central limitation that a real Mysterious Treasure in hand can have zero AMR despite looking connected in a graph.

### Matchup-flex JIT

The payload itself remains JIT. Cards whose value is visibly low in a specified matchup become discard candidates. The policy makes Mawile-GX, Dipplin, some reactive Supporters, Field Blower, stadiums, recovery cards, and Powerglass eligible when their matchup job is not presently required. Goodra VSTAR can also become eligible when an S-tier payload remains available and defensive damage reduction is not the selected plan.

### No-discard-control

A/S payloads can enter the discard before the attack turn. This produces an optimistic non-control reference line, not a recommendation for every match.

## Lock scenarios

| Scenario | Meaning |
|---|---|
| None | Baseline goldfish condition. |
| Turn-2 item lock | Turn 1 is normal. Item cards cannot be played from turn 2 onward. This is the registered current-paper Item-lock timing. |
| Full Rule Box ability lock | Rule Box Pokémon Abilities are unavailable. The model treats Forest Seal Stone as usable through Path to the Peak because the VSTAR Power is on the Tool. |
| Combined | Rule Box Pokémon Abilities are suppressed from the start, while Item cards remain legal on turn 1 and become locked from turn 2 onward. This uses the same Item timing as the Turn-2 item-lock scenario. |

`LockMode::FullItem` remains only for focused synthetic or historical regression fixtures and has no registered aggregate or trace label. The current-paper aggregate omits turn-one full Item-lock rows because the starting player cannot attack on the first turn and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

“Lock with no outs” is a stress-test condition. Channeler is not treated as a generic answer because its text removes effects of attacks on your Pokémon. It cannot generally remove an opponent’s ongoing Item or Ability lock.

## Policy design

The policy follows observable information only.

1. Put Regidrago V into play as soon as a legal connector can do it.
2. Preserve the Active-position axis through a starting Regidrago V or Latias ex.
3. Use legal discard connectors only when a DCI candidate exists under the selected profile.
4. Before evolving, use Forest Seal Stone on Regidrago V when it resolves the highest missing axis. Going second on turn 1, an incomplete VSTAR/Crispin/Blender package directs Forest Seal Stone toward Steven’s Resolve.
5. Select Crispin on an attack-relevant turn when the GGF energy axis is missing.
6. Select Steven’s Resolve on going-second turn 1 where a Regidrago V and the one manual Energy attachment already exist, then fetch VSTAR, Crispin, and Blender.
7. Use Arven for Blender plus Forest Seal Stone when the package is stronger than the available direct Supporter.
8. Evolve only when legal.
9. Delay strict-JIT payload disposal until the successful attack turn. Prefer Blender for a deck payload, then a hand-discard Item/Supporter where its required primary action is still legal.
10. Promote the evolved Regidrago with Latias ex when the current Active is a Basic and the Ability is available.

This is a policy search with rules-enforced transitions. It does not use a future-card oracle. A literal near-perfect solver would need a partially observable, two-player stochastic game model with opponent archetype distributions and a much larger rule database.

## Success predicates

### Setup-ready

A trial is setup-ready on turn `t` when all statements are true:

1. The Active Pokémon is Regidrago VSTAR.
2. It has at least two attached Grass Energy and one attached Fire Energy.
3. The discard pile contains Dragapult ex, Mega Dragonite ex, Dialga-GX, Hisuian Goodra VSTAR, or recipe-gated Appletun `sv8-140` when the selected registered recipe contains it: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/swsh12-136 https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_001.inc#L128-L137 https://github.com/FlareZ123/pokemon-sims/issues/1489
4. In strict and matchup-flex JIT modes, at least one such payload moved to the discard during turn `t`.
5. `t >= 2`.

Dragapult ex and Mega Dragonite ex are S-tier. Dialga-GX’s Timeless-GX and Hisuian Goodra VSTAR’s Rolling Iron are A-tier in the supplied tier list.

### Reported metrics

- `P(ready by T2)`, `P(ready by T3)`, `P(ready by T4)`, and cumulative `P(ready by T5)`.
- `P(ready on T5)` records diagnostic late recovery; those games remain setup failures.
- `setup_failure_pct` counts every game not ready by the end of T4 and equals `100 - ready_by_t4_pct`: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7-L12 https://github.com/FlareZ123/pokemon-sims/issues/1491
- Monte Carlo standard error in percentage points for baseline scenario values.
- Setup-tool use rates and opening Regidrago V rate.
- Registered-deck scenario probabilities for `regidrago-shell` and `regidrago-pineco`, including the paired `--all-decks` artifact: https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv https://github.com/FlareZ123/pokemon-sims/issues/1493
- Source-bound paired Crobat V card-swap deltas for T2, T3, and T4 readiness, scenario improvements, Dark Asset use, and cards drawn per using game. The `--model-crobat` generator writes `results/crobat_variant_model.csv`, `--model-variant` reproduces one readable hand, and `docs/CROBAT_MODEL_REPORT.md` records discrete cut costs and interpretation boundaries: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://api.pokemontcg.io/v2/cards/swsh3-104 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1496

## Validation plan

1. Audit all exact requested card IDs against the supplied corpus.
2. Assert 18 Pokémon, 33 Trainers, 9 Energy, and 60 total cards, derived from the canonical decklist and card-audit contract: https://github.com/FlareZ123/pokemon-sims/blob/main/data/decklist.json https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/audit_card_data.py
3. Unit-test deck total and category totals.
4. Build with warnings enabled.
5. Run `ctest`.
6. Smoke-test both registered recipes and byte-compare the aggregate `--all-decks` matrix. Separately regenerate the modeling-only Crobat V matrix, validate one `--model-variant` trace, and keep the retired generic `variant_results.csv` excluded: https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml#L133-L140 https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#model-crobat-v-swaps https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CROBAT_MODEL_REPORT.md https://github.com/FlareZ123/pokemon-sims/blob/main/results/README.md#L7 https://github.com/FlareZ123/pokemon-sims/issues/1394 https://github.com/FlareZ123/pokemon-sims/issues/1493 https://github.com/FlareZ123/pokemon-sims/issues/1496
7. Run a fixed-seed large trial count for reproducible final CSV files.
8. Inspect any implausible result by checking timing gates, Supporter contention, and same-turn discard markers.
9. Record known omissions before interpreting the percentages.

## Planned refinements after this project

- Add a belief-state evaluator that estimates expected payoff of draw Supporters rather than using the current fallback priority.
- Add a richer opponent model with named locks, damage clocks, Iono/Judge-style hand disruption, gust, and prize-taking.
- Add explicit damage-board analysis for Phantom Dive, Ryuno Glide, Rolling Iron, Timeless-GX, Dipplin, and Powerglass loops.
- Add alternative-card effect modules only after their exact text and Live availability are audited.

## Battle VIP Pass extension

Battle VIP Pass is represented as a first-turn-only Item connector with a two-Basic direct-to-Bench cap, K1 transition, shuffle, Bench-space enforcement, played-from-hand Ability suppression, static-Ability continuity, and post-turn-one dead-card DCI. Production support is independent of deck registration. Validation uses a temporary shell derivative outside the named registry: https://api.pokemontcg.io/v2/cards/swsh8-225 https://github.com/FlareZ123/pokemon-sims/issues/1647
