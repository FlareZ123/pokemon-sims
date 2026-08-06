# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.060% | 19.422% | +7.362 pp | 40.047% | 47.990% | +7.943 pp | 56.973% | 66.003% | +9.030 pp |
| Strict JIT, going second | 29.873% | 47.564% | +17.691 pp | 53.743% | 63.241% | +9.498 pp | 65.287% | 73.825% | +8.538 pp |
| Matchup-flex JIT, going first | 16.327% | 21.024% | +4.697 pp | 48.599% | 49.930% | +1.331 pp | 64.979% | 68.358% | +3.379 pp |
| Matchup-flex JIT, going second | 37.164% | 50.894% | +13.730 pp | 61.518% | 66.807% | +5.289 pp | 72.143% | 76.441% | +4.298 pp |
| No discard control, going first | 19.958% | 25.216% | +5.258 pp | 56.020% | 60.418% | +4.398 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.123% | 73.939% | +6.816 pp | 78.368% | 82.494% | +4.126 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.060% ± 0.103 | 40.047% ± 0.155 | 56.973% ± 0.157 | 43.027% ± 0.157 |
| Matchup-flex JIT, going first | 16.327% ± 0.117 | 48.599% ± 0.158 | 64.979% ± 0.151 | 35.021% ± 0.151 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.393% ± 0.097 | 18.262% ± 0.122 | 81.738% ± 0.122 |
| Strict JIT, full Item lock, first | 2.850% ± 0.053 | 7.890% ± 0.085 | 15.425% ± 0.114 | 84.575% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.408% ± 0.065 | 26.530% ± 0.140 | 40.311% ± 0.155 | 59.689% ± 0.155 |
| Strict JIT, combined lock, first | 0.308% ± 0.018 | 3.371% ± 0.057 | 7.514% ± 0.083 | 92.486% ± 0.083 |
| Strict JIT, going second | 29.873% ± 0.145 | 53.743% ± 0.158 | 65.287% ± 0.151 | 34.713% ± 0.151 |
| Matchup-flex JIT, going second | 37.164% ± 0.153 | 61.518% ± 0.154 | 72.143% ± 0.142 | 27.857% ± 0.142 |
| No discard control, going second | 39.964% ± 0.155 | 67.123% ± 0.149 | 78.368% ± 0.130 | 21.632% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.177% ± 0.110 | 28.393% ± 0.143 | 36.916% ± 0.153 | 63.084% ± 0.153 |
| Strict JIT, full Item lock, second | 10.515% ± 0.097 | 23.371% ± 0.134 | 31.330% ± 0.147 | 68.670% ± 0.147 |
| Strict JIT, Rule Box Ability lock, second | 18.299% ± 0.122 | 35.535% ± 0.151 | 46.340% ± 0.158 | 53.660% ± 0.158 |
| Strict JIT, combined lock, second | 2.510% ± 0.049 | 11.587% ± 0.101 | 16.268% ± 0.117 | 83.732% ± 0.117 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.422% ± 0.114 | 21.773% ± 0.131 | 78.227% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.120% ± 0.086 | 19.416% ± 0.125 | 25.348% ± 0.138 | 74.652% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.060% | 27.987% | 16.926% | 10.759% |
| Matchup-flex JIT, going first | 16.327% | 32.272% | 16.380% | 9.749% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.873% | 23.870% | 11.544% | 7.911% |
| Matchup-flex JIT, going second | 37.164% | 24.354% | 10.625% | 7.093% |
| No discard control, going second | 39.964% | 27.159% | 11.245% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.422% ± 0.125 | 47.990% ± 0.158 | 66.003% ± 0.150 | 33.997% ± 0.150 |
| Matchup-flex JIT, going first | 21.024% ± 0.129 | 49.930% ± 0.158 | 68.358% ± 0.147 | 31.642% ± 0.147 |
| No discard control, going first | 25.216% ± 0.137 | 60.418% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.673% ± 0.067 | 8.673% ± 0.089 | 14.888% ± 0.113 | 85.112% ± 0.113 |
| Strict JIT, full Item lock, first | 2.870% ± 0.053 | 6.105% ± 0.076 | 11.355% ± 0.100 | 88.645% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.943% ± 0.069 | 18.317% ± 0.122 | 30.470% ± 0.146 | 69.530% ± 0.146 |
| Strict JIT, combined lock, first | 0.502% ± 0.022 | 1.448% ± 0.038 | 3.288% ± 0.056 | 96.712% ± 0.056 |
| Strict JIT, going second | 47.564% ± 0.158 | 63.241% ± 0.152 | 73.825% ± 0.139 | 26.175% ± 0.139 |
| Matchup-flex JIT, going second | 50.894% ± 0.158 | 66.807% ± 0.149 | 76.441% ± 0.134 | 23.559% ± 0.134 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.543% ± 0.066 | 9.476% ± 0.093 | 15.035% ± 0.113 | 84.965% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.305% ± 0.104 | 24.307% ± 0.136 | 33.999% ± 0.150 | 66.001% ± 0.150 |
| Strict JIT, combined lock, second | 1.220% ± 0.035 | 2.774% ± 0.052 | 4.741% ± 0.067 | 95.259% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.924% ± 0.043 | 6.895% ± 0.080 | 11.923% ± 0.102 | 88.077% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.376% ± 0.077 | 11.196% ± 0.100 | 16.342% ± 0.117 | 83.658% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.422% | 28.568% | 18.013% | 10.634% |
| Matchup-flex JIT, going first | 21.024% | 28.906% | 18.428% | 10.380% |
| No discard control, going first | 25.216% | 35.202% | 15.387% | 7.192% |
| Strict JIT, going second | 47.564% | 15.677% | 10.584% | 6.946% |
| Matchup-flex JIT, going second | 50.894% | 15.913% | 9.634% | 6.600% |
| No discard control, going second | 59.434% | 14.505% | 8.555% | 4.635% |

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
| Secret Box use | 64.301% |
| Exploding Energy use | 78.576% |
| Steven use | 35.811% |
| Star Alchemy use | 48.117% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.047 per game |
| Missing route axis | 0.774 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.301 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.401 |
| Pineco/Forretress line | 0.425 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.471 |
| Discard zone | 0.238 |
| Stranded hand zone | 0.189 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `06ac34179ea16d3683f453cac508040acc458d789b38386924b01a124cf316ea`.

Comparison CSV SHA-256: `e31ee9bf9b5310b14e39f2746b9e7ce678919a7f1a42adafdcbb0b39fcb1f6ad`.
