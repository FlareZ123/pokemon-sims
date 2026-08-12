# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% | 19.627% | +7.343 pp | 41.895% | 49.020% | +7.125 pp | 59.173% | 66.561% | +7.388 pp |
| Strict JIT, going second | 29.873% | 48.228% | +18.355 pp | 55.181% | 64.103% | +8.922 pp | 67.292% | 74.380% | +7.088 pp |
| Matchup-flex JIT, going first | 17.030% | 21.121% | +4.091 pp | 50.426% | 50.679% | +0.253 pp | 67.307% | 68.697% | +1.390 pp |
| Matchup-flex JIT, going second | 37.292% | 51.458% | +14.166 pp | 63.197% | 67.114% | +3.917 pp | 74.539% | 76.863% | +2.324 pp |
| No discard control, going first | 19.770% | 25.259% | +5.489 pp | 56.731% | 60.275% | +3.544 pp | 73.456% | 75.722% | +2.266 pp |
| No discard control, going second | 40.269% | 59.291% | +19.022 pp | 68.437% | 73.785% | +5.348 pp | 79.955% | 82.360% | +2.405 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% ± 0.104 | 41.895% ± 0.156 | 59.173% ± 0.155 | 40.827% ± 0.155 |
| Matchup-flex JIT, going first | 17.030% ± 0.119 | 50.426% ± 0.158 | 67.307% ± 0.148 | 32.693% ± 0.148 |
| No discard control, going first | 19.770% ± 0.126 | 56.731% ± 0.157 | 73.456% ± 0.140 | 26.544% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.548% ± 0.066 | 10.389% ± 0.096 | 18.380% ± 0.122 | 81.620% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.527% ± 0.066 | 27.128% ± 0.141 | 41.330% ± 0.156 | 58.670% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.770% ± 0.028 | 4.606% ± 0.066 | 9.569% ± 0.093 | 90.431% ± 0.093 |
| Strict JIT, going second | 29.873% ± 0.145 | 55.181% ± 0.157 | 67.292% ± 0.148 | 32.708% ± 0.148 |
| Matchup-flex JIT, going second | 37.292% ± 0.153 | 63.197% ± 0.153 | 74.539% ± 0.138 | 25.461% ± 0.138 |
| No discard control, going second | 40.269% ± 0.155 | 68.437% ± 0.147 | 79.955% ± 0.127 | 20.045% ± 0.127 |
| Strict JIT, turn-two Item lock, second | 14.056% ± 0.110 | 28.291% ± 0.142 | 36.990% ± 0.153 | 63.010% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.382% ± 0.122 | 36.669% ± 0.152 | 48.409% ± 0.158 | 51.591% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.798% ± 0.060 | 14.428% ± 0.111 | 19.916% ± 0.126 | 80.084% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.020% ± 0.113 | 22.025% ± 0.131 | 77.975% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.011% ± 0.086 | 19.400% ± 0.125 | 25.858% ± 0.138 | 74.142% ± 0.138 |
| Garbodor + Boost Shake Ability lock, first | 5.564% ± 0.072 | 26.973% ± 0.140 | 40.832% ± 0.155 | 59.168% ± 0.155 |
| Garbodor + Boost Shake Ability lock, second | 17.335% ± 0.120 | 34.473% ± 0.150 | 46.309% ± 0.158 | 53.691% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% | 29.611% | 17.278% | 10.901% |
| Matchup-flex JIT, going first | 17.030% | 33.396% | 16.881% | 9.824% |
| No discard control, going first | 19.770% | 36.961% | 16.725% | 9.058% |
| Strict JIT, going second | 29.873% | 25.308% | 12.111% | 7.914% |
| Matchup-flex JIT, going second | 37.292% | 25.905% | 11.342% | 7.083% |
| No discard control, going second | 40.269% | 28.168% | 11.518% | 6.385% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.627% ± 0.126 | 49.020% ± 0.158 | 66.561% ± 0.149 | 33.439% ± 0.149 |
| Matchup-flex JIT, going first | 21.121% ± 0.129 | 50.679% ± 0.158 | 68.697% ± 0.147 | 31.303% ± 0.147 |
| No discard control, going first | 25.259% ± 0.137 | 60.275% ± 0.155 | 75.722% ± 0.136 | 24.278% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.674% ± 0.067 | 8.639% ± 0.089 | 14.877% ± 0.113 | 85.123% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.172% ± 0.070 | 19.393% ± 0.125 | 31.767% ± 0.147 | 68.233% ± 0.147 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.169% ± 0.034 | 2.677% ± 0.051 | 5.404% ± 0.071 | 94.596% ± 0.071 |
| Strict JIT, going second | 48.228% ± 0.158 | 64.103% ± 0.152 | 74.380% ± 0.138 | 25.620% ± 0.138 |
| Matchup-flex JIT, going second | 51.458% ± 0.158 | 67.114% ± 0.149 | 76.863% ± 0.133 | 23.137% ± 0.133 |
| No discard control, going second | 59.291% ± 0.155 | 73.785% ± 0.139 | 82.360% ± 0.121 | 17.640% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.869% ± 0.080 | 12.825% ± 0.106 | 19.154% ± 0.124 | 80.846% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.128% ± 0.107 | 25.639% ± 0.138 | 35.723% ± 0.152 | 64.277% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.974% ± 0.044 | 4.642% ± 0.067 | 7.330% ± 0.082 | 92.670% ± 0.082 |
| Strict JIT, Supporter lock, first | 1.868% ± 0.043 | 7.515% ± 0.083 | 12.762% ± 0.106 | 87.238% ± 0.106 |
| Strict JIT, Supporter lock, second | 6.499% ± 0.078 | 11.749% ± 0.102 | 17.034% ± 0.119 | 82.966% ± 0.119 |
| Garbodor + Boost Shake Ability lock, first | 6.957% ± 0.080 | 21.509% ± 0.130 | 33.701% ± 0.149 | 66.299% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 12.875% ± 0.106 | 25.375% ± 0.138 | 35.333% ± 0.151 | 64.667% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.627% | 29.393% | 17.541% | 10.515% |
| Matchup-flex JIT, going first | 21.121% | 29.558% | 18.018% | 10.233% |
| No discard control, going first | 25.259% | 35.016% | 15.447% | 7.220% |
| Strict JIT, going second | 48.228% | 15.875% | 10.277% | 6.780% |
| Matchup-flex JIT, going second | 51.458% | 15.656% | 9.749% | 6.298% |
| No discard control, going second | 59.291% | 14.494% | 8.575% | 4.667% |

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
| Secret Box use | 64.318% |
| Exploding Energy use | 78.484% |
| Steven use | 35.749% |
| Star Alchemy use | 48.088% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.048 per game |
| Missing route axis | 0.772 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.301 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.396 |
| Pineco/Forretress line | 0.425 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.026 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.473 |
| Discard zone | 0.236 |
| Stranded hand zone | 0.191 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `72fd5a7d11e1e65350a16bd105f03fec46b913532214387a1757af90eaa7e5ae`.

Comparison CSV SHA-256: `61435b6eee2e3771f3a11e09db392b5210ba0cf53a66b825444b6496f1faa011`.