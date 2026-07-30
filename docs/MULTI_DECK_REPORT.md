# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.063% | 19.665% | +7.602 pp | 39.787% | 48.358% | +8.571 pp | 56.639% | 66.155% | +9.516 pp |
| Strict JIT, going second | 29.650% | 47.655% | +18.005 pp | 53.509% | 63.147% | +9.638 pp | 64.837% | 73.726% | +8.889 pp |
| Matchup-flex JIT, going first | 16.485% | 21.081% | +4.596 pp | 48.206% | 50.006% | +1.800 pp | 64.077% | 68.262% | +4.185 pp |
| Matchup-flex JIT, going second | 37.234% | 51.341% | +14.107 pp | 61.206% | 66.856% | +5.650 pp | 71.488% | 76.501% | +5.013 pp |
| No discard control, going first | 19.985% | 25.200% | +5.215 pp | 56.040% | 60.129% | +4.089 pp | 72.375% | 75.679% | +3.304 pp |
| No discard control, going second | 39.973% | 59.583% | +19.610 pp | 67.119% | 74.005% | +6.886 pp | 78.400% | 82.436% | +4.036 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.063% ± 0.103 | 39.787% ± 0.155 | 56.639% ± 0.157 | 43.361% ± 0.157 |
| Matchup-flex JIT, going first | 16.485% ± 0.117 | 48.206% ± 0.158 | 64.077% ± 0.152 | 35.923% ± 0.152 |
| No discard control, going first | 19.985% ± 0.126 | 56.040% ± 0.157 | 72.375% ± 0.141 | 27.625% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.264% ± 0.064 | 26.372% ± 0.139 | 40.097% ± 0.155 | 59.903% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.360% ± 0.057 | 7.469% ± 0.083 | 92.531% ± 0.083 |
| Strict JIT, going second | 29.650% ± 0.144 | 53.509% ± 0.158 | 64.837% ± 0.151 | 35.163% ± 0.151 |
| Matchup-flex JIT, going second | 37.234% ± 0.153 | 61.206% ± 0.154 | 71.488% ± 0.143 | 28.512% ± 0.143 |
| No discard control, going second | 39.973% ± 0.155 | 67.119% ± 0.149 | 78.400% ± 0.130 | 21.600% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.221% ± 0.110 | 28.110% ± 0.142 | 35.873% ± 0.152 | 64.127% ± 0.152 |
| Strict JIT, full Item lock, second | 10.564% ± 0.097 | 23.252% ± 0.134 | 30.573% ± 0.146 | 69.427% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.413% ± 0.123 | 35.591% ± 0.151 | 45.968% ± 0.158 | 54.032% ± 0.158 |
| Strict JIT, combined lock, second | 2.510% ± 0.049 | 11.514% ± 0.101 | 15.884% ± 0.116 | 84.116% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.435% ± 0.114 | 21.716% ± 0.130 | 78.284% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.165% ± 0.087 | 19.466% ± 0.125 | 25.347% ± 0.138 | 74.653% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.063% | 27.724% | 16.852% | 10.656% |
| Matchup-flex JIT, going first | 16.485% | 31.721% | 15.871% | 9.529% |
| No discard control, going first | 19.985% | 36.055% | 16.335% | 9.000% |
| Strict JIT, going second | 29.650% | 23.859% | 11.328% | 7.905% |
| Matchup-flex JIT, going second | 37.234% | 23.972% | 10.282% | 6.951% |
| No discard control, going second | 39.973% | 27.146% | 11.281% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.665% ± 0.126 | 48.358% ± 0.158 | 66.155% ± 0.150 | 33.845% ± 0.150 |
| Matchup-flex JIT, going first | 21.081% ± 0.129 | 50.006% ± 0.158 | 68.262% ± 0.147 | 31.738% ± 0.147 |
| No discard control, going first | 25.200% ± 0.137 | 60.129% ± 0.155 | 75.679% ± 0.136 | 24.321% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.697% ± 0.067 | 8.691% ± 0.089 | 14.846% ± 0.112 | 85.154% ± 0.112 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.890% ± 0.068 | 18.220% ± 0.122 | 30.438% ± 0.146 | 69.562% ± 0.146 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.655% ± 0.158 | 63.147% ± 0.153 | 73.726% ± 0.139 | 26.274% ± 0.139 |
| Matchup-flex JIT, going second | 51.341% ± 0.158 | 66.856% ± 0.149 | 76.501% ± 0.134 | 23.499% ± 0.134 |
| No discard control, going second | 59.583% ± 0.155 | 74.005% ± 0.139 | 82.436% ± 0.120 | 17.564% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.904% ± 0.080 | 12.837% ± 0.106 | 19.124% ± 0.124 | 80.876% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.355% ± 0.104 | 24.257% ± 0.136 | 34.063% ± 0.150 | 65.937% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.866% ± 0.043 | 6.916% ± 0.080 | 11.898% ± 0.102 | 88.102% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.304% ± 0.077 | 11.201% ± 0.100 | 16.280% ± 0.117 | 83.720% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.665% | 28.693% | 17.797% | 10.461% |
| Matchup-flex JIT, going first | 21.081% | 28.925% | 18.256% | 10.365% |
| No discard control, going first | 25.200% | 34.929% | 15.550% | 7.163% |
| Strict JIT, going second | 47.655% | 15.492% | 10.579% | 7.039% |
| Matchup-flex JIT, going second | 51.341% | 15.515% | 9.645% | 6.527% |
| No discard control, going second | 59.583% | 14.422% | 8.431% | 4.606% |

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
| Secret Box use | 64.363% |
| Exploding Energy use | 78.618% |
| Steven use | 35.767% |
| Star Alchemy use | 48.109% |
| Secret Box attempts | 1.474 per game |
| Cost blocks | 0.049 per game |
| Missing route axis | 0.780 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.046 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.433 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.155 |
| Known Prize zone | 0.478 |
| Discard zone | 0.240 |
| Stranded hand zone | 0.192 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `da11c6fd69194d2bb2fe18e09175a0cc3096f88e8e4645a524f47403916e382e`.

Comparison CSV SHA-256: `b4377abbd24501ccf1b378f8265ab2c7c91fff2902c13ada4d13362b932495a5`.
