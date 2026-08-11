# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 4.214% | 2.034% | -2.180 pp | 30.645% | 24.463% | -6.182 pp | 61.757% | 54.628% | -7.129 pp |
| Strict JIT, going second | 23.898% | 11.854% | -12.044 pp | 46.090% | 36.485% | -9.605 pp | 72.598% | 65.478% | -7.120 pp |
| Matchup-flex JIT, going first | 4.268% | 2.034% | -2.234 pp | 30.918% | 24.463% | -6.455 pp | 62.063% | 54.628% | -7.435 pp |
| Matchup-flex JIT, going second | 24.027% | 11.854% | -12.173 pp | 46.278% | 36.485% | -9.793 pp | 72.773% | 65.478% | -7.295 pp |
| No discard control, going first | 4.649% | 2.034% | -2.615 pp | 37.196% | 24.463% | -12.733 pp | 70.974% | 54.628% | -16.346 pp |
| No discard control, going second | 27.042% | 11.854% | -15.188 pp | 52.196% | 36.485% | -15.711 pp | 79.205% | 65.478% | -13.727 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 4.214% ± 0.064 | 30.645% ± 0.146 | 61.757% ± 0.154 | 2.820% ± 0.052 |
| Matchup-flex JIT, going first | 4.268% ± 0.064 | 30.918% ± 0.146 | 62.063% ± 0.153 | 2.749% ± 0.052 |
| No discard control, going first | 4.649% ± 0.067 | 37.196% ± 0.153 | 70.974% ± 0.144 | 1.740% ± 0.041 |
| Strict JIT, turn-two Item lock, first | 0.300% ± 0.017 | 5.983% ± 0.075 | 29.487% ± 0.144 | 5.136% ± 0.070 |
| Strict JIT, Rule Box Ability lock, first | 2.762% ± 0.052 | 18.669% ± 0.123 | 45.709% ± 0.158 | 6.176% ± 0.076 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.153% ± 0.012 | 3.255% ± 0.056 | 18.006% ± 0.121 | 10.010% ± 0.095 |
| Strict JIT, going second | 23.898% ± 0.135 | 46.090% ± 0.158 | 72.598% ± 0.141 | 2.116% ± 0.045 |
| Matchup-flex JIT, going second | 24.027% ± 0.135 | 46.278% ± 0.158 | 72.773% ± 0.141 | 2.045% ± 0.045 |
| No discard control, going second | 27.042% ± 0.140 | 52.196% ± 0.158 | 79.205% ± 0.128 | 1.312% ± 0.036 |
| Strict JIT, turn-two Item lock, second | 4.054% ± 0.062 | 16.386% ± 0.117 | 48.736% ± 0.158 | 4.473% ± 0.065 |
| Strict JIT, Rule Box Ability lock, second | 16.474% ± 0.117 | 33.534% ± 0.149 | 61.415% ± 0.154 | 5.082% ± 0.069 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.253% ± 0.035 | 8.334% ± 0.087 | 30.443% ± 0.146 | 8.821% ± 0.090 |
| Strict JIT, Supporter lock, first | 4.214% ± 0.064 | 15.205% ± 0.114 | 35.875% ± 0.152 | 11.693% ± 0.102 |
| Strict JIT, Supporter lock, second | 23.898% ± 0.135 | 34.202% ± 0.150 | 55.283% ± 0.157 | 8.727% ± 0.089 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 4.214% | 26.431% | 31.112% | 25.474% |
| Matchup-flex JIT, going first | 4.268% | 26.650% | 31.145% | 25.241% |
| No discard control, going first | 4.649% | 32.547% | 33.778% | 21.579% |
| Strict JIT, going second | 23.898% | 22.192% | 26.508% | 15.718% |
| Matchup-flex JIT, going second | 24.027% | 22.251% | 26.495% | 15.730% |
| No discard control, going second | 27.042% | 25.154% | 27.009% | 14.035% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 2.034% ± 0.045 | 24.463% ± 0.136 | 54.628% ± 0.157 | 5.695% ± 0.073 |
| Matchup-flex JIT, going first | 2.034% ± 0.045 | 24.463% ± 0.136 | 54.628% ± 0.157 | 5.695% ± 0.073 |
| No discard control, going first | 2.034% ± 0.045 | 24.463% ± 0.136 | 54.628% ± 0.157 | 5.695% ± 0.073 |
| Strict JIT, turn-two Item lock, first | 0.085% ± 0.009 | 4.492% ± 0.065 | 22.530% ± 0.132 | 10.239% ± 0.096 |
| Strict JIT, Rule Box Ability lock, first | 1.689% ± 0.041 | 16.033% ± 0.116 | 41.262% ± 0.156 | 9.194% ± 0.091 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.026% ± 0.005 | 2.760% ± 0.052 | 16.150% ± 0.116 | 13.948% ± 0.110 |
| Strict JIT, going second | 11.854% ± 0.102 | 36.485% ± 0.152 | 65.478% ± 0.150 | 4.804% ± 0.068 |
| Matchup-flex JIT, going second | 11.854% ± 0.102 | 36.485% ± 0.152 | 65.478% ± 0.150 | 4.804% ± 0.068 |
| No discard control, going second | 11.854% ± 0.102 | 36.485% ± 0.152 | 65.478% ± 0.150 | 4.804% ± 0.068 |
| Strict JIT, turn-two Item lock, second | 2.467% ± 0.049 | 12.860% ± 0.106 | 42.322% ± 0.156 | 8.083% ± 0.086 |
| Strict JIT, Rule Box Ability lock, second | 8.761% ± 0.089 | 26.479% ± 0.140 | 55.375% ± 0.157 | 8.203% ± 0.087 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 0.506% ± 0.022 | 6.143% ± 0.076 | 25.382% ± 0.138 | 12.756% ± 0.105 |
| Strict JIT, Supporter lock, first | 2.034% ± 0.045 | 13.237% ± 0.107 | 31.228% ± 0.146 | 15.853% ± 0.115 |
| Strict JIT, Supporter lock, second | 11.854% ± 0.102 | 26.292% ± 0.139 | 47.589% ± 0.158 | 12.794% ± 0.106 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 2.034% | 22.429% | 30.165% | 24.365% |
| Matchup-flex JIT, going first | 2.034% | 22.429% | 30.165% | 24.365% |
| No discard control, going first | 2.034% | 22.429% | 30.165% | 24.365% |
| Strict JIT, going second | 11.854% | 24.631% | 28.993% | 19.718% |
| Matchup-flex JIT, going second | 11.854% | 24.631% | 28.993% | 19.718% |
| No discard control, going second | 11.854% | 24.631% | 28.993% | 19.718% |

## Secret Box route graph

```mermaid
graph LR
  MT[Mysterious Treasure] --> Tapu[Tapu Lele-GX]
  QB[Quick Ball] --> Tapu
  Tapu --> Arven
  Tapu --> Gladion
  Arven --> Box[Secret Box]
  Gladion -->|known prized| Box
  Steven[Steven's Resolve] --> Box
  FSS[Forest Seal Stone] --> Box
  Box --> Item[Mysterious Treasure or replacement Item]
  Box --> Tool[Forest Seal Stone when Fire is missing]
  Box --> Dawn
  Box --> Forest[Forest of Vitality when immediate evolution is needed]
  Dawn --> Pineco
  Dawn --> Forretress[Forretress ex]
  Dawn --> Payload[Dragon payload]
  Item --> VSTAR[Regidrago VSTAR]
  Tool --> Fire[Fire Energy]
  Forretress --> Grass[Grass Energy]
  Payload --> Discard[Strict-JIT discard]
  VSTAR --> Ready[Apex Dragon ready]
  Fire --> Ready
  Grass --> Ready
  Discard --> Ready
```

The graph is adaptive. Held cards, prior-turn setup, legal Prize knowledge, and ordinary evolution can remove a search category. The policy reserves every additional discard cost before paying Secret Box.

## Route-frequency diagnostics

The following row is `regidrago-pineco`, no-discard-control, going second. Counts may overlap because one rejected state can miss several axes.

| Route metric | Value |
|---|---:|
| Secret Box use | 9.405% |
| Exploding Energy use | 0.570% |
| Steven use | 2.755% |
| Star Alchemy use | 7.649% |
| Secret Box attempts | 0.145 per game |
| Cost blocks | 0.051 per game |
| Missing route axis | 0.038 per game |
| Bench blocks | 0.000 per game |
| Arven banks | 0.000 per game |
| Steven banks | 0.000 per game |
| Gladion banks | 0.000 per game |
| FSS banks | 0.000 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.040 |
| Pineco/Forretress line | 0.044 |
| VSTAR | 0.015 |
| Payload | 0.007 |
| Search Item | 0.011 |
| Fire | 0.003 |
| Grass | 0.001 |
| Ability | 0.000 |
| Supporter | 0.000 |
| Known Prize zone | 0.007 |
| Discard zone | 0.017 |
| Stranded hand zone | 0.003 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `261d1c41e2706aa85dffd0adf5851a7460004df37e916b7d6df42f81d1e0a227`.

Comparison CSV SHA-256: `029cf21d77e1a045017db09e525d3a73fe1d9b9f81a558e4b6b2e0d838239b78`.
