# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.269% | 18.355% | +7.086 pp | 37.479% | 44.282% | +6.803 pp | 54.237% | 61.628% | +7.391 pp |
| Strict JIT, going second | 28.278% | 45.726% | +17.448 pp | 51.538% | 60.711% | +9.173 pp | 62.983% | 71.327% | +8.344 pp |
| Matchup-flex JIT, going first | 16.245% | 19.927% | +3.682 pp | 47.211% | 45.573% | -1.638 pp | 63.173% | 63.328% | +0.155 pp |
| Matchup-flex JIT, going second | 36.532% | 48.929% | +12.397 pp | 60.485% | 64.005% | +3.520 pp | 70.904% | 73.569% | +2.665 pp |
| No discard control, going first | 19.963% | 24.697% | +4.734 pp | 55.349% | 57.826% | +2.477 pp | 71.524% | 73.590% | +2.066 pp |
| No discard control, going second | 39.949% | 58.515% | +18.566 pp | 67.000% | 72.807% | +5.807 pp | 78.492% | 81.319% | +2.827 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.269% ± 0.100 | 37.479% ± 0.153 | 54.237% ± 0.158 | 45.763% ± 0.158 |
| Matchup-flex JIT, going first | 16.245% ± 0.117 | 47.211% ± 0.158 | 63.173% ± 0.153 | 36.827% ± 0.153 |
| No discard control, going first | 19.963% ± 0.126 | 55.349% ± 0.157 | 71.524% ± 0.143 | 28.476% ± 0.143 |
| Strict JIT, turn-two Item lock, first | 4.614% ± 0.066 | 10.317% ± 0.096 | 17.743% ± 0.121 | 82.257% ± 0.121 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 7.457% ± 0.083 | 14.725% ± 0.112 | 85.275% ± 0.112 |
| Strict JIT, Rule Box Ability lock, first | 4.314% ± 0.064 | 25.501% ± 0.138 | 38.551% ± 0.154 | 61.449% ± 0.154 |
| Strict JIT, combined lock, first | 0.282% ± 0.017 | 3.243% ± 0.056 | 7.154% ± 0.081 | 92.846% ± 0.081 |
| Strict JIT, going second | 28.278% ± 0.142 | 51.538% ± 0.158 | 62.983% ± 0.153 | 37.017% ± 0.153 |
| Matchup-flex JIT, going second | 36.532% ± 0.152 | 60.485% ± 0.155 | 70.904% ± 0.144 | 29.096% ± 0.144 |
| No discard control, going second | 39.949% ± 0.155 | 67.000% ± 0.149 | 78.492% ± 0.130 | 21.508% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 13.864% ± 0.109 | 27.435% ± 0.141 | 35.054% ± 0.151 | 64.946% ± 0.151 |
| Strict JIT, full Item lock, second | 10.606% ± 0.097 | 22.843% ± 0.133 | 30.026% ± 0.145 | 69.974% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.919% ± 0.121 | 34.356% ± 0.150 | 44.571% ± 0.157 | 55.429% ± 0.157 |
| Strict JIT, combined lock, second | 2.360% ± 0.048 | 11.398% ± 0.100 | 15.487% ± 0.114 | 84.513% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 10.006% ± 0.095 | 16.674% ± 0.118 | 83.326% ± 0.118 |
| Strict JIT, Supporter lock, second | 5.804% ± 0.074 | 13.997% ± 0.110 | 20.025% ± 0.127 | 79.975% ± 0.127 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.269% | 26.210% | 16.758% | 10.729% |
| Matchup-flex JIT, going first | 16.245% | 30.966% | 15.962% | 9.782% |
| No discard control, going first | 19.963% | 35.386% | 16.175% | 9.033% |
| Strict JIT, going second | 28.278% | 23.260% | 11.445% | 7.845% |
| Matchup-flex JIT, going second | 36.532% | 23.953% | 10.419% | 6.927% |
| No discard control, going second | 39.949% | 27.051% | 11.492% | 6.484% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.355% ± 0.122 | 44.282% ± 0.157 | 61.628% ± 0.154 | 38.372% ± 0.154 |
| Matchup-flex JIT, going first | 19.927% ± 0.126 | 45.573% ± 0.157 | 63.328% ± 0.152 | 36.672% ± 0.152 |
| No discard control, going first | 24.697% ± 0.136 | 57.826% ± 0.156 | 73.590% ± 0.139 | 26.410% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.616% ± 0.066 | 7.953% ± 0.086 | 13.608% ± 0.108 | 86.392% ± 0.108 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.712% ± 0.073 | 11.041% ± 0.099 | 88.959% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.434% ± 0.072 | 19.186% ± 0.125 | 31.851% ± 0.147 | 68.149% ± 0.147 |
| Strict JIT, combined lock, first | 0.639% ± 0.025 | 1.862% ± 0.043 | 4.287% ± 0.064 | 95.713% ± 0.064 |
| Strict JIT, going second | 45.726% ± 0.158 | 60.711% ± 0.154 | 71.327% ± 0.143 | 28.673% ± 0.143 |
| Matchup-flex JIT, going second | 48.929% ± 0.158 | 64.005% ± 0.152 | 73.569% ± 0.139 | 26.431% ± 0.139 |
| No discard control, going second | 58.515% ± 0.156 | 72.807% ± 0.141 | 81.319% ± 0.123 | 18.681% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.727% ± 0.079 | 12.186% ± 0.103 | 18.038% ± 0.122 | 81.962% ± 0.122 |
| Strict JIT, full Item lock, second | 4.676% ± 0.067 | 9.522% ± 0.093 | 15.053% ± 0.113 | 84.947% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.236% ± 0.107 | 25.720% ± 0.138 | 36.226% ± 0.152 | 63.774% ± 0.152 |
| Strict JIT, combined lock, second | 1.383% ± 0.037 | 3.309% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.068% ± 0.033 | 5.822% ± 0.074 | 9.952% ± 0.095 | 90.048% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.987% ± 0.069 | 9.652% ± 0.093 | 13.787% ± 0.109 | 86.213% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.355% | 25.927% | 17.346% | 10.523% |
| Matchup-flex JIT, going first | 19.927% | 25.646% | 17.755% | 10.347% |
| No discard control, going first | 24.697% | 33.129% | 15.764% | 8.156% |
| Strict JIT, going second | 45.726% | 14.985% | 10.616% | 6.696% |
| Matchup-flex JIT, going second | 48.929% | 15.076% | 9.564% | 6.378% |
| No discard control, going second | 58.515% | 14.292% | 8.512% | 4.973% |

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
| Secret Box use | 62.650% |
| Exploding Energy use | 82.964% |
| Steven use | 36.459% |
| Star Alchemy use | 46.390% |
| Secret Box attempts | 1.564 per game |
| Cost blocks | 0.051 per game |
| Missing route axis | 0.885 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.309 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.499 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.030 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.170 |
| Known Prize zone | 0.532 |
| Discard zone | 0.302 |
| Stranded hand zone | 0.185 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `e88be27b391716b25554556a6d39073c4aa528e48aca7fe1cf2744565ffe4bb0`.

Comparison CSV SHA-256: `8745f7b694c489e2cacf506c4f9a9a3ac733b4fb3b6fc00698244864b7cfaced`.
