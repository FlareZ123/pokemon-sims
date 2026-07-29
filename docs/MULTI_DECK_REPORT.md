# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.024% | 19.633% | +7.609 pp | 39.075% | 47.912% | +8.837 pp | 56.131% | 65.630% | +9.499 pp |
| Strict JIT, going second | 29.515% | 47.475% | +17.960 pp | 53.230% | 62.841% | +9.611 pp | 64.565% | 73.607% | +9.042 pp |
| Matchup-flex JIT, going first | 16.404% | 21.013% | +4.609 pp | 47.715% | 49.167% | +1.452 pp | 63.562% | 67.425% | +3.863 pp |
| Matchup-flex JIT, going second | 37.328% | 51.042% | +13.714 pp | 61.056% | 66.447% | +5.391 pp | 71.391% | 76.147% | +4.756 pp |
| No discard control, going first | 20.013% | 25.361% | +5.348 pp | 56.018% | 60.065% | +4.047 pp | 72.215% | 75.670% | +3.455 pp |
| No discard control, going second | 39.944% | 59.653% | +19.709 pp | 67.090% | 74.024% | +6.934 pp | 78.237% | 82.430% | +4.193 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.024% ± 0.103 | 39.075% ± 0.154 | 56.131% ± 0.157 | 43.869% ± 0.157 |
| Matchup-flex JIT, going first | 16.404% ± 0.117 | 47.715% ± 0.158 | 63.562% ± 0.152 | 36.438% ± 0.152 |
| No discard control, going first | 20.013% ± 0.127 | 56.018% ± 0.157 | 72.215% ± 0.142 | 27.785% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.643% ± 0.067 | 10.398% ± 0.097 | 18.136% ± 0.122 | 81.864% ± 0.122 |
| Strict JIT, full Item lock, first | 2.753% ± 0.052 | 7.685% ± 0.084 | 15.072% ± 0.113 | 84.928% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.344% ± 0.064 | 26.386% ± 0.139 | 39.701% ± 0.155 | 60.299% ± 0.155 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.309% ± 0.057 | 7.394% ± 0.083 | 92.606% ± 0.083 |
| Strict JIT, going second | 29.515% ± 0.144 | 53.230% ± 0.158 | 64.565% ± 0.151 | 35.435% ± 0.151 |
| Matchup-flex JIT, going second | 37.328% ± 0.153 | 61.056% ± 0.154 | 71.391% ± 0.143 | 28.609% ± 0.143 |
| No discard control, going second | 39.944% ± 0.155 | 67.090% ± 0.149 | 78.237% ± 0.130 | 21.763% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.262% ± 0.111 | 28.249% ± 0.142 | 35.905% ± 0.152 | 64.095% ± 0.152 |
| Strict JIT, full Item lock, second | 10.627% ± 0.097 | 23.371% ± 0.134 | 30.600% ± 0.146 | 69.400% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 17.946% ± 0.121 | 35.215% ± 0.151 | 45.717% ± 0.158 | 54.283% ± 0.158 |
| Strict JIT, combined lock, second | 2.376% ± 0.048 | 11.343% ± 0.100 | 15.623% ± 0.115 | 84.377% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.327% ± 0.114 | 21.605% ± 0.130 | 78.395% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.192% ± 0.087 | 19.690% ± 0.126 | 25.501% ± 0.138 | 74.499% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.024% | 27.051% | 17.056% | 10.742% |
| Matchup-flex JIT, going first | 16.404% | 31.311% | 15.847% | 9.633% |
| No discard control, going first | 20.013% | 36.005% | 16.197% | 8.999% |
| Strict JIT, going second | 29.515% | 23.715% | 11.335% | 7.857% |
| Matchup-flex JIT, going second | 37.328% | 23.728% | 10.335% | 6.883% |
| No discard control, going second | 39.944% | 27.146% | 11.147% | 6.550% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.633% ± 0.126 | 47.912% ± 0.158 | 65.630% ± 0.150 | 34.370% ± 0.150 |
| Matchup-flex JIT, going first | 21.013% ± 0.129 | 49.167% ± 0.158 | 67.425% ± 0.148 | 32.575% ± 0.148 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.697% ± 0.067 | 8.691% ± 0.089 | 14.846% ± 0.112 | 85.154% ± 0.112 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.897% ± 0.068 | 18.224% ± 0.122 | 30.318% ± 0.145 | 69.682% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.475% ± 0.158 | 62.841% ± 0.153 | 73.607% ± 0.139 | 26.393% ± 0.139 |
| Matchup-flex JIT, going second | 51.042% ± 0.158 | 66.447% ± 0.149 | 76.147% ± 0.135 | 23.853% ± 0.135 |
| No discard control, going second | 59.653% ± 0.155 | 74.024% ± 0.139 | 82.430% ± 0.120 | 17.570% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.904% ± 0.080 | 12.837% ± 0.106 | 19.124% ± 0.124 | 80.876% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.292% ± 0.104 | 24.134% ± 0.135 | 33.809% ± 0.150 | 66.191% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.853% ± 0.043 | 6.899% ± 0.080 | 11.867% ± 0.102 | 88.133% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.303% ± 0.077 | 11.189% ± 0.100 | 16.301% ± 0.117 | 83.699% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.633% | 28.279% | 17.718% | 10.941% |
| Matchup-flex JIT, going first | 21.013% | 28.154% | 18.258% | 10.648% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.475% | 15.366% | 10.766% | 6.959% |
| Matchup-flex JIT, going second | 51.042% | 15.405% | 9.700% | 6.476% |
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

Simulator policy digest: `99bd1c41ac13119126f489b8f219c15620c59e4941f83b46c96e0abe7dfb0998`.

Comparison CSV SHA-256: `290047f8e1b390454fef5c047914297590dbd57d1a2fa3a70a2b9e53c8aa2233`.
