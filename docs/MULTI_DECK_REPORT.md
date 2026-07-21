# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.032% | 18.579% | +6.547 pp | 37.987% | 44.189% | +6.202 pp | 54.666% | 61.800% | +7.134 pp |
| Strict JIT, going second | 28.476% | 46.084% | +17.608 pp | 51.490% | 60.812% | +9.322 pp | 62.919% | 71.218% | +8.299 pp |
| Matchup-flex JIT, going first | 16.292% | 19.982% | +3.690 pp | 47.246% | 45.630% | -1.616 pp | 63.185% | 63.434% | +0.249 pp |
| Matchup-flex JIT, going second | 36.544% | 48.584% | +12.040 pp | 60.489% | 63.718% | +3.229 pp | 70.908% | 73.527% | +2.619 pp |
| No discard control, going first | 19.963% | 24.491% | +4.528 pp | 55.349% | 57.666% | +2.317 pp | 71.524% | 73.649% | +2.125 pp |
| No discard control, going second | 39.949% | 58.374% | +18.425 pp | 67.000% | 72.601% | +5.601 pp | 78.492% | 81.145% | +2.653 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.032% ± 0.103 | 37.987% ± 0.153 | 54.666% ± 0.157 | 45.334% ± 0.157 |
| Matchup-flex JIT, going first | 16.292% ± 0.117 | 47.246% ± 0.158 | 63.185% ± 0.153 | 36.815% ± 0.153 |
| No discard control, going first | 19.963% ± 0.126 | 55.349% ± 0.157 | 71.524% ± 0.143 | 28.476% ± 0.143 |
| Strict JIT, turn-two Item lock, first | 4.614% ± 0.066 | 10.317% ± 0.096 | 17.743% ± 0.121 | 82.257% ± 0.121 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 7.457% ± 0.083 | 14.725% ± 0.112 | 85.275% ± 0.112 |
| Strict JIT, Rule Box Ability lock, first | 4.456% ± 0.065 | 25.613% ± 0.138 | 38.690% ± 0.154 | 61.310% ± 0.154 |
| Strict JIT, combined lock, first | 0.282% ± 0.017 | 3.243% ± 0.056 | 7.154% ± 0.081 | 92.846% ± 0.081 |
| Strict JIT, going second | 28.476% ± 0.143 | 51.490% ± 0.158 | 62.919% ± 0.153 | 37.081% ± 0.153 |
| Matchup-flex JIT, going second | 36.544% ± 0.152 | 60.489% ± 0.155 | 70.908% ± 0.144 | 29.092% ± 0.144 |
| No discard control, going second | 39.949% ± 0.155 | 67.000% ± 0.149 | 78.492% ± 0.130 | 21.508% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 13.864% ± 0.109 | 27.435% ± 0.141 | 35.054% ± 0.151 | 64.946% ± 0.151 |
| Strict JIT, full Item lock, second | 10.606% ± 0.097 | 22.843% ± 0.133 | 30.026% ± 0.145 | 69.974% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.890% ± 0.121 | 34.398% ± 0.150 | 44.553% ± 0.157 | 55.447% ± 0.157 |
| Strict JIT, combined lock, second | 2.360% ± 0.048 | 11.398% ± 0.100 | 15.487% ± 0.114 | 84.513% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 10.062% ± 0.095 | 16.692% ± 0.118 | 83.308% ± 0.118 |
| Strict JIT, Supporter lock, second | 5.830% ± 0.074 | 14.094% ± 0.110 | 20.166% ± 0.127 | 79.834% ± 0.127 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.032% | 25.955% | 16.679% | 10.640% |
| Matchup-flex JIT, going first | 16.292% | 30.954% | 15.939% | 9.767% |
| No discard control, going first | 19.963% | 35.386% | 16.175% | 9.033% |
| Strict JIT, going second | 28.476% | 23.014% | 11.429% | 7.926% |
| Matchup-flex JIT, going second | 36.544% | 23.945% | 10.419% | 6.929% |
| No discard control, going second | 39.949% | 27.051% | 11.492% | 6.484% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.579% ± 0.123 | 44.189% ± 0.157 | 61.800% ± 0.154 | 38.200% ± 0.154 |
| Matchup-flex JIT, going first | 19.982% ± 0.126 | 45.630% ± 0.158 | 63.434% ± 0.152 | 36.566% ± 0.152 |
| No discard control, going first | 24.491% ± 0.136 | 57.666% ± 0.156 | 73.649% ± 0.139 | 26.351% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.706% ± 0.067 | 7.986% ± 0.086 | 13.602% ± 0.108 | 86.398% ± 0.108 |
| Strict JIT, full Item lock, first | 2.784% ± 0.052 | 5.742% ± 0.074 | 11.124% ± 0.099 | 88.876% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.370% ± 0.071 | 19.132% ± 0.124 | 32.077% ± 0.148 | 67.923% ± 0.148 |
| Strict JIT, combined lock, first | 0.634% ± 0.025 | 1.863% ± 0.043 | 4.300% ± 0.064 | 95.700% ± 0.064 |
| Strict JIT, going second | 46.084% ± 0.158 | 60.812% ± 0.154 | 71.218% ± 0.143 | 28.782% ± 0.143 |
| Matchup-flex JIT, going second | 48.584% ± 0.158 | 63.718% ± 0.152 | 73.527% ± 0.140 | 26.473% ± 0.140 |
| No discard control, going second | 58.374% ± 0.156 | 72.601% ± 0.141 | 81.145% ± 0.124 | 18.855% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.710% ± 0.079 | 12.105% ± 0.103 | 17.884% ± 0.121 | 82.116% ± 0.121 |
| Strict JIT, full Item lock, second | 4.705% ± 0.067 | 9.544% ± 0.093 | 15.105% ± 0.113 | 84.895% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.147% ± 0.107 | 25.623% ± 0.138 | 36.271% ± 0.152 | 63.729% ± 0.152 |
| Strict JIT, combined lock, second | 1.374% ± 0.037 | 3.302% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.124% ± 0.033 | 5.932% ± 0.075 | 10.035% ± 0.095 | 89.965% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.965% ± 0.069 | 9.664% ± 0.093 | 13.837% ± 0.109 | 86.163% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.579% | 25.610% | 17.611% | 10.560% |
| Matchup-flex JIT, going first | 19.982% | 25.648% | 17.804% | 10.453% |
| No discard control, going first | 24.491% | 33.175% | 15.983% | 7.971% |
| Strict JIT, going second | 46.084% | 14.728% | 10.406% | 6.770% |
| Matchup-flex JIT, going second | 48.584% | 15.134% | 9.809% | 6.388% |
| No discard control, going second | 58.374% | 14.227% | 8.544% | 5.083% |

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
| Secret Box use | 62.554% |
| Exploding Energy use | 83.009% |
| Steven use | 36.187% |
| Star Alchemy use | 46.536% |
| Secret Box attempts | 1.567 per game |
| Cost blocks | 0.050 per game |
| Missing route axis | 0.889 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.305 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.500 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.029 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.170 |
| Known Prize zone | 0.531 |
| Discard zone | 0.305 |
| Stranded hand zone | 0.183 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `4ffc3b9746f03e3b4052fabc1c21258b657aa2ccb3e2f623c26629c78d370b75`.

Comparison CSV SHA-256: `eca7ae2792c9dacef7458a18f89b5def7b5512d8defc0ac01d858a883d2adb7c`.
