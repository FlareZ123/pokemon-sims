# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.032% | 18.497% | +6.465 pp | 37.987% | 44.495% | +6.508 pp | 54.666% | 61.797% | +7.131 pp |
| Strict JIT, going second | 28.476% | 46.147% | +17.671 pp | 51.490% | 60.850% | +9.360 pp | 62.919% | 71.326% | +8.407 pp |
| Matchup-flex JIT, going first | 16.245% | 19.917% | +3.672 pp | 47.211% | 45.709% | -1.502 pp | 63.173% | 63.513% | +0.340 pp |
| Matchup-flex JIT, going second | 36.544% | 48.712% | +12.168 pp | 60.489% | 63.847% | +3.358 pp | 70.908% | 73.582% | +2.674 pp |
| No discard control, going first | 19.963% | 24.653% | +4.690 pp | 55.349% | 57.635% | +2.286 pp | 71.524% | 73.515% | +1.991 pp |
| No discard control, going second | 39.949% | 58.498% | +18.549 pp | 67.000% | 72.710% | +5.710 pp | 78.492% | 81.226% | +2.734 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.032% ± 0.103 | 37.987% ± 0.153 | 54.666% ± 0.157 | 45.334% ± 0.157 |
| Matchup-flex JIT, going first | 16.245% ± 0.117 | 47.211% ± 0.158 | 63.173% ± 0.153 | 36.827% ± 0.153 |
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
| Matchup-flex JIT, going first | 16.245% | 30.966% | 15.962% | 9.782% |
| No discard control, going first | 19.963% | 35.386% | 16.175% | 9.033% |
| Strict JIT, going second | 28.476% | 23.014% | 11.429% | 7.926% |
| Matchup-flex JIT, going second | 36.544% | 23.945% | 10.419% | 6.929% |
| No discard control, going second | 39.949% | 27.051% | 11.492% | 6.484% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.497% ± 0.123 | 44.495% ± 0.157 | 61.797% ± 0.154 | 38.203% ± 0.154 |
| Matchup-flex JIT, going first | 19.917% ± 0.126 | 45.709% ± 0.158 | 63.513% ± 0.152 | 36.487% ± 0.152 |
| No discard control, going first | 24.653% ± 0.136 | 57.635% ± 0.156 | 73.515% ± 0.140 | 26.485% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.721% ± 0.067 | 8.001% ± 0.086 | 13.635% ± 0.109 | 86.365% ± 0.109 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.712% ± 0.073 | 11.041% ± 0.099 | 88.959% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.359% ± 0.071 | 19.252% ± 0.125 | 32.061% ± 0.148 | 67.939% ± 0.148 |
| Strict JIT, combined lock, first | 0.639% ± 0.025 | 1.862% ± 0.043 | 4.287% ± 0.064 | 95.713% ± 0.064 |
| Strict JIT, going second | 46.147% ± 0.158 | 60.850% ± 0.154 | 71.326% ± 0.143 | 28.674% ± 0.143 |
| Matchup-flex JIT, going second | 48.712% ± 0.158 | 63.847% ± 0.152 | 73.582% ± 0.139 | 26.418% ± 0.139 |
| No discard control, going second | 58.498% ± 0.156 | 72.710% ± 0.141 | 81.226% ± 0.123 | 18.774% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.712% ± 0.079 | 12.121% ± 0.103 | 17.967% ± 0.121 | 82.033% ± 0.121 |
| Strict JIT, full Item lock, second | 4.676% ± 0.067 | 9.522% ± 0.093 | 15.053% ± 0.113 | 84.947% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.211% ± 0.107 | 25.712% ± 0.138 | 36.249% ± 0.152 | 63.751% ± 0.152 |
| Strict JIT, combined lock, second | 1.383% ± 0.037 | 3.309% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.124% ± 0.033 | 5.935% ± 0.075 | 10.040% ± 0.095 | 89.960% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.968% ± 0.069 | 9.670% ± 0.093 | 13.845% ± 0.109 | 86.155% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.497% | 25.998% | 17.302% | 10.532% |
| Matchup-flex JIT, going first | 19.917% | 25.792% | 17.804% | 10.458% |
| No discard control, going first | 24.653% | 32.982% | 15.880% | 8.120% |
| Strict JIT, going second | 46.147% | 14.703% | 10.476% | 6.673% |
| Matchup-flex JIT, going second | 48.712% | 15.135% | 9.735% | 6.309% |
| No discard control, going second | 58.498% | 14.212% | 8.516% | 5.022% |

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
| Secret Box use | 62.491% |
| Exploding Energy use | 82.938% |
| Steven use | 36.195% |
| Star Alchemy use | 46.351% |
| Secret Box attempts | 1.559 per game |
| Cost blocks | 0.051 per game |
| Missing route axis | 0.883 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.306 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.393 |
| Pineco/Forretress line | 0.499 |
| VSTAR | 0.004 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.030 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.169 |
| Known Prize zone | 0.529 |
| Discard zone | 0.303 |
| Stranded hand zone | 0.183 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `fbe553cf76117d3a9e56f8a7679b3e748d02490c33ce6b87cd16f23fb360f47d`.

Comparison CSV SHA-256: `c919130abf0e720da999f186fdd2185cedb0a3c76db325b22c69d6d8c97fa3af`.
