# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.075% | 19.616% | +7.541 pp | 39.144% | 47.906% | +8.762 pp | 56.332% | 65.633% | +9.301 pp |
| Strict JIT, going second | 29.548% | 47.473% | +17.925 pp | 53.164% | 62.843% | +9.679 pp | 64.633% | 73.603% | +8.970 pp |
| Matchup-flex JIT, going first | 16.469% | 21.020% | +4.551 pp | 47.651% | 49.173% | +1.522 pp | 63.566% | 67.420% | +3.854 pp |
| Matchup-flex JIT, going second | 37.264% | 51.063% | +13.799 pp | 61.063% | 66.446% | +5.383 pp | 71.376% | 76.141% | +4.765 pp |
| No discard control, going first | 19.985% | 25.361% | +5.376 pp | 56.040% | 60.065% | +4.025 pp | 72.375% | 75.670% | +3.295 pp |
| No discard control, going second | 39.971% | 59.653% | +19.682 pp | 67.119% | 74.024% | +6.905 pp | 78.400% | 82.430% | +4.030 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.075% ± 0.103 | 39.144% ± 0.154 | 56.332% ± 0.157 | 43.668% ± 0.157 |
| Matchup-flex JIT, going first | 16.469% ± 0.117 | 47.651% ± 0.158 | 63.566% ± 0.152 | 36.434% ± 0.152 |
| No discard control, going first | 19.985% ± 0.126 | 56.040% ± 0.157 | 72.375% ± 0.141 | 27.625% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.780% ± 0.052 | 7.725% ± 0.084 | 15.148% ± 0.113 | 84.852% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.333% ± 0.064 | 26.501% ± 0.140 | 39.890% ± 0.155 | 60.110% ± 0.155 |
| Strict JIT, combined lock, first | 0.299% ± 0.017 | 3.258% ± 0.056 | 7.392% ± 0.083 | 92.608% ± 0.083 |
| Strict JIT, going second | 29.548% ± 0.144 | 53.164% ± 0.158 | 64.633% ± 0.151 | 35.367% ± 0.151 |
| Matchup-flex JIT, going second | 37.264% ± 0.153 | 61.063% ± 0.154 | 71.376% ± 0.143 | 28.624% ± 0.143 |
| No discard control, going second | 39.971% ± 0.155 | 67.119% ± 0.149 | 78.400% ± 0.130 | 21.600% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.221% ± 0.110 | 28.110% ± 0.142 | 35.873% ± 0.152 | 64.127% ± 0.152 |
| Strict JIT, full Item lock, second | 10.611% ± 0.097 | 23.221% ± 0.134 | 30.445% ± 0.146 | 69.555% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 17.988% ± 0.121 | 35.376% ± 0.151 | 45.947% ± 0.158 | 54.053% ± 0.158 |
| Strict JIT, combined lock, second | 2.415% ± 0.049 | 11.435% ± 0.101 | 15.762% ± 0.115 | 84.238% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.327% ± 0.114 | 21.605% ± 0.130 | 78.395% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.192% ± 0.087 | 19.690% ± 0.126 | 25.501% ± 0.138 | 74.499% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.075% | 27.069% | 17.188% | 10.652% |
| Matchup-flex JIT, going first | 16.469% | 31.182% | 15.915% | 9.666% |
| No discard control, going first | 19.985% | 36.055% | 16.335% | 9.000% |
| Strict JIT, going second | 29.548% | 23.616% | 11.469% | 7.853% |
| Matchup-flex JIT, going second | 37.264% | 23.799% | 10.313% | 6.909% |
| No discard control, going second | 39.971% | 27.148% | 11.281% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% ± 0.126 | 47.906% ± 0.158 | 65.633% ± 0.150 | 34.367% ± 0.150 |
| Matchup-flex JIT, going first | 21.020% ± 0.129 | 49.173% ± 0.158 | 67.420% ± 0.148 | 32.580% ± 0.148 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.697% ± 0.067 | 8.691% ± 0.089 | 14.846% ± 0.112 | 85.154% ± 0.112 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.897% ± 0.068 | 18.222% ± 0.122 | 30.316% ± 0.145 | 69.684% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.473% ± 0.158 | 62.843% ± 0.153 | 73.603% ± 0.139 | 26.397% ± 0.139 |
| Matchup-flex JIT, going second | 51.063% ± 0.158 | 66.446% ± 0.149 | 76.141% ± 0.135 | 23.859% ± 0.135 |
| No discard control, going second | 59.653% ± 0.155 | 74.024% ± 0.139 | 82.430% ± 0.120 | 17.570% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.904% ± 0.080 | 12.837% ± 0.106 | 19.124% ± 0.124 | 80.876% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.292% ± 0.104 | 24.142% ± 0.135 | 33.813% ± 0.150 | 66.187% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.853% ± 0.043 | 6.899% ± 0.080 | 11.867% ± 0.102 | 88.133% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.303% ± 0.077 | 11.189% ± 0.100 | 16.301% ± 0.117 | 83.699% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% | 28.290% | 17.727% | 10.944% |
| Matchup-flex JIT, going first | 21.020% | 28.153% | 18.247% | 10.646% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.473% | 15.370% | 10.760% | 6.960% |
| Matchup-flex JIT, going second | 51.063% | 15.383% | 9.695% | 6.473% |
| No discard control, going second | 59.653% | 14.371% | 8.406% | 4.598% |

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
| Secret Box use | 64.384% |
| Exploding Energy use | 78.624% |
| Steven use | 35.752% |
| Star Alchemy use | 48.074% |
| Secret Box attempts | 1.471 per game |
| Cost blocks | 0.048 per game |
| Missing route axis | 0.778 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.397 |
| Pineco/Forretress line | 0.431 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.155 |
| Known Prize zone | 0.477 |
| Discard zone | 0.238 |
| Stranded hand zone | 0.191 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `03565f20bd400d0a34f26db3214e683be603ee7eb4ff426bf5b95d8665b34899`.

Comparison CSV SHA-256: `dde9039b8a4f35c5ea99716d45abfa9e7deb8fce67b98cde30c5062588434798`.
