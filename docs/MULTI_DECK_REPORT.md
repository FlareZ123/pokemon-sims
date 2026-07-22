# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% | 18.392% | +6.423 pp | 38.000% | 43.926% | +5.926 pp | 54.941% | 61.431% | +6.490 pp |
| Strict JIT, going second | 28.815% | 46.140% | +17.325 pp | 51.980% | 60.774% | +8.794 pp | 63.341% | 70.952% | +7.611 pp |
| Matchup-flex JIT, going first | 16.406% | 19.920% | +3.514 pp | 47.384% | 45.183% | -2.201 pp | 63.398% | 62.557% | -0.841 pp |
| Matchup-flex JIT, going second | 37.228% | 49.014% | +11.786 pp | 60.742% | 63.920% | +3.178 pp | 71.195% | 73.321% | +2.126 pp |
| No discard control, going first | 19.954% | 24.520% | +4.566 pp | 55.774% | 57.533% | +1.759 pp | 71.845% | 73.262% | +1.417 pp |
| No discard control, going second | 39.779% | 58.532% | +18.753 pp | 66.746% | 72.500% | +5.754 pp | 78.054% | 80.777% | +2.723 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% ± 0.103 | 38.000% ± 0.153 | 54.941% ± 0.157 | 45.059% ± 0.157 |
| Matchup-flex JIT, going first | 16.406% ± 0.117 | 47.384% ± 0.158 | 63.398% ± 0.152 | 36.602% ± 0.152 |
| No discard control, going first | 19.954% ± 0.126 | 55.774% ± 0.157 | 71.845% ± 0.142 | 28.155% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.582% ± 0.066 | 10.187% ± 0.096 | 17.684% ± 0.121 | 82.316% ± 0.121 |
| Strict JIT, full Item lock, first | 2.820% ± 0.052 | 7.733% ± 0.084 | 15.056% ± 0.113 | 84.944% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.463% ± 0.065 | 25.882% ± 0.139 | 38.851% ± 0.154 | 61.149% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 28.815% ± 0.143 | 51.980% ± 0.158 | 63.341% ± 0.152 | 36.659% ± 0.152 |
| Matchup-flex JIT, going second | 37.228% ± 0.153 | 60.742% ± 0.154 | 71.195% ± 0.143 | 28.805% ± 0.143 |
| No discard control, going second | 39.779% ± 0.155 | 66.746% ± 0.149 | 78.054% ± 0.131 | 21.946% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.187% ± 0.110 | 28.035% ± 0.142 | 35.673% ± 0.151 | 64.327% ± 0.151 |
| Strict JIT, full Item lock, second | 10.495% ± 0.097 | 22.877% ± 0.133 | 30.039% ± 0.145 | 69.961% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.961% ± 0.121 | 34.490% ± 0.150 | 44.668% ± 0.157 | 55.332% ± 0.157 |
| Strict JIT, combined lock, second | 2.352% ± 0.048 | 11.342% ± 0.100 | 15.418% ± 0.114 | 84.582% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 10.163% ± 0.096 | 16.908% ± 0.119 | 83.092% ± 0.119 |
| Strict JIT, Supporter lock, second | 6.165% ± 0.076 | 14.616% ± 0.112 | 20.911% ± 0.129 | 79.089% ± 0.129 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% | 26.031% | 16.941% | 10.678% |
| Matchup-flex JIT, going first | 16.406% | 30.978% | 16.014% | 9.551% |
| No discard control, going first | 19.954% | 35.820% | 16.071% | 9.002% |
| Strict JIT, going second | 28.815% | 23.165% | 11.361% | 7.811% |
| Matchup-flex JIT, going second | 37.228% | 23.514% | 10.453% | 7.082% |
| No discard control, going second | 39.779% | 26.967% | 11.308% | 6.657% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.392% ± 0.123 | 43.926% ± 0.157 | 61.431% ± 0.154 | 38.569% ± 0.154 |
| Matchup-flex JIT, going first | 19.920% ± 0.126 | 45.183% ± 0.157 | 62.557% ± 0.153 | 37.443% ± 0.153 |
| No discard control, going first | 24.520% ± 0.136 | 57.533% ± 0.156 | 73.262% ± 0.140 | 26.738% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.527% ± 0.066 | 7.707% ± 0.084 | 13.293% ± 0.107 | 86.707% ± 0.107 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.672% ± 0.073 | 10.711% ± 0.098 | 89.289% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 5.115% ± 0.070 | 18.949% ± 0.124 | 31.678% ± 0.147 | 68.322% ± 0.147 |
| Strict JIT, combined lock, first | 0.581% ± 0.024 | 1.732% ± 0.041 | 4.038% ± 0.062 | 95.962% ± 0.062 |
| Strict JIT, going second | 46.140% ± 0.158 | 60.774% ± 0.154 | 70.952% ± 0.144 | 29.048% ± 0.144 |
| Matchup-flex JIT, going second | 49.014% ± 0.158 | 63.920% ± 0.152 | 73.321% ± 0.140 | 26.679% ± 0.140 |
| No discard control, going second | 58.532% ± 0.156 | 72.500% ± 0.141 | 80.777% ± 0.125 | 19.223% ± 0.125 |
| Strict JIT, turn-two Item lock, second | 6.597% ± 0.078 | 11.702% ± 0.102 | 17.356% ± 0.120 | 82.644% ± 0.120 |
| Strict JIT, full Item lock, second | 4.367% ± 0.065 | 9.104% ± 0.091 | 14.482% ± 0.111 | 85.518% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 13.073% ± 0.107 | 25.497% ± 0.138 | 35.731% ± 0.152 | 64.269% ± 0.152 |
| Strict JIT, combined lock, second | 1.383% ± 0.037 | 3.268% ± 0.056 | 5.595% ± 0.073 | 94.405% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.096% ± 0.033 | 5.962% ± 0.075 | 10.043% ± 0.095 | 89.957% ± 0.095 |
| Strict JIT, Supporter lock, second | 5.023% ± 0.069 | 9.488% ± 0.093 | 13.746% ± 0.109 | 86.254% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.392% | 25.534% | 17.505% | 10.533% |
| Matchup-flex JIT, going first | 19.920% | 25.263% | 17.374% | 10.500% |
| No discard control, going first | 24.520% | 33.013% | 15.729% | 7.743% |
| Strict JIT, going second | 46.140% | 14.634% | 10.178% | 6.518% |
| Matchup-flex JIT, going second | 49.014% | 14.906% | 9.401% | 6.017% |
| No discard control, going second | 58.532% | 13.968% | 8.277% | 4.927% |

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
| Secret Box use | 62.792% |
| Exploding Energy use | 77.830% |
| Steven use | 36.230% |
| Star Alchemy use | 47.870% |
| Secret Box attempts | 1.551 per game |
| Cost blocks | 0.054 per game |
| Missing route axis | 0.868 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.307 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.043 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.400 |
| Pineco/Forretress line | 0.473 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.029 |
| Grass | 0.003 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.510 |
| Discard zone | 0.278 |
| Stranded hand zone | 0.172 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `3767a878927b02d1e3583c3890c445946555022dab277f9352adbda7cf06d006`.

Comparison CSV SHA-256: `fac4d3786d777b08f82692d8ed44636462455eb2ad6e25225979eb200c787514`.
