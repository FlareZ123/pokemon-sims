# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.808% | 18.392% | +6.584 pp | 38.046% | 43.926% | +5.880 pp | 54.987% | 61.431% | +6.444 pp |
| Strict JIT, going second | 28.853% | 46.140% | +17.287 pp | 52.174% | 60.774% | +8.600 pp | 63.490% | 70.952% | +7.462 pp |
| Matchup-flex JIT, going first | 16.174% | 19.920% | +3.746 pp | 47.248% | 45.183% | -2.065 pp | 63.136% | 62.557% | -0.579 pp |
| Matchup-flex JIT, going second | 36.753% | 49.014% | +12.261 pp | 60.649% | 63.920% | +3.271 pp | 71.109% | 73.321% | +2.212 pp |
| No discard control, going first | 19.931% | 24.520% | +4.589 pp | 55.711% | 57.533% | +1.822 pp | 71.713% | 73.262% | +1.549 pp |
| No discard control, going second | 39.845% | 58.532% | +18.687 pp | 66.994% | 72.500% | +5.506 pp | 78.455% | 80.777% | +2.322 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.808% ± 0.102 | 38.046% ± 0.154 | 54.987% ± 0.157 | 45.013% ± 0.157 |
| Matchup-flex JIT, going first | 16.174% ± 0.116 | 47.248% ± 0.158 | 63.136% ± 0.153 | 36.864% ± 0.153 |
| No discard control, going first | 19.931% ± 0.126 | 55.711% ± 0.157 | 71.713% ± 0.142 | 28.287% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.582% ± 0.066 | 10.187% ± 0.096 | 17.684% ± 0.121 | 82.316% ± 0.121 |
| Strict JIT, full Item lock, first | 2.820% ± 0.052 | 7.733% ± 0.084 | 15.056% ± 0.113 | 84.944% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.360% ± 0.065 | 25.688% ± 0.138 | 38.826% ± 0.154 | 61.174% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 28.853% ± 0.143 | 52.174% ± 0.158 | 63.490% ± 0.152 | 36.510% ± 0.152 |
| Matchup-flex JIT, going second | 36.753% ± 0.152 | 60.649% ± 0.154 | 71.109% ± 0.143 | 28.891% ± 0.143 |
| No discard control, going second | 39.845% ± 0.155 | 66.994% ± 0.149 | 78.455% ± 0.130 | 21.545% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.187% ± 0.110 | 28.035% ± 0.142 | 35.673% ± 0.151 | 64.327% ± 0.151 |
| Strict JIT, full Item lock, second | 10.495% ± 0.097 | 22.877% ± 0.133 | 30.039% ± 0.145 | 69.961% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.002% ± 0.121 | 34.556% ± 0.150 | 44.866% ± 0.157 | 55.134% ± 0.157 |
| Strict JIT, combined lock, second | 2.352% ± 0.048 | 11.342% ± 0.100 | 15.418% ± 0.114 | 84.582% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 10.065% ± 0.095 | 16.990% ± 0.119 | 83.010% ± 0.119 |
| Strict JIT, Supporter lock, second | 6.112% ± 0.076 | 14.528% ± 0.111 | 20.991% ± 0.129 | 79.009% ± 0.129 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.808% | 26.238% | 16.941% | 10.724% |
| Matchup-flex JIT, going first | 16.174% | 31.074% | 15.888% | 9.761% |
| No discard control, going first | 19.931% | 35.780% | 16.002% | 9.028% |
| Strict JIT, going second | 28.853% | 23.321% | 11.316% | 7.861% |
| Matchup-flex JIT, going second | 36.753% | 23.896% | 10.460% | 7.047% |
| No discard control, going second | 39.845% | 27.149% | 11.461% | 6.548% |

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

Simulator policy digest: `1b8559b14d5fc4b706a3a061f4223baa8ecac2f945538d9f33f9eab0594420df`.

Comparison CSV SHA-256: `c860d0b32481538954cd680ad5f76fb30337c621c1ca433c67be1add8ab8064d`.
