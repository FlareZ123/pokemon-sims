# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% | 18.590% | +6.621 pp | 38.000% | 44.352% | +6.352 pp | 54.941% | 61.740% | +6.799 pp |
| Strict JIT, going second | 28.729% | 46.342% | +17.613 pp | 51.976% | 61.184% | +9.208 pp | 63.346% | 71.400% | +8.054 pp |
| Matchup-flex JIT, going first | 16.406% | 20.055% | +3.649 pp | 47.384% | 45.331% | -2.053 pp | 63.398% | 62.794% | -0.604 pp |
| Matchup-flex JIT, going second | 37.104% | 49.351% | +12.247 pp | 60.856% | 63.969% | +3.113 pp | 71.247% | 73.500% | +2.253 pp |
| No discard control, going first | 19.954% | 24.730% | +4.776 pp | 55.774% | 57.809% | +2.035 pp | 71.845% | 73.389% | +1.544 pp |
| No discard control, going second | 39.987% | 58.537% | +18.550 pp | 66.854% | 72.764% | +5.910 pp | 78.162% | 81.128% | +2.966 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% ± 0.103 | 38.000% ± 0.153 | 54.941% ± 0.157 | 45.059% ± 0.157 |
| Matchup-flex JIT, going first | 16.406% ± 0.117 | 47.384% ± 0.158 | 63.398% ± 0.152 | 36.602% ± 0.152 |
| No discard control, going first | 19.954% ± 0.126 | 55.774% ± 0.157 | 71.845% ± 0.142 | 28.155% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.582% ± 0.066 | 10.187% ± 0.096 | 17.684% ± 0.121 | 82.316% ± 0.121 |
| Strict JIT, full Item lock, first | 2.820% ± 0.052 | 7.733% ± 0.084 | 15.056% ± 0.113 | 84.944% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.459% ± 0.065 | 25.873% ± 0.138 | 38.850% ± 0.154 | 61.150% ± 0.154 |
| Strict JIT, combined lock, first | 0.292% ± 0.017 | 3.275% ± 0.056 | 7.263% ± 0.082 | 92.737% ± 0.082 |
| Strict JIT, going second | 28.729% ± 0.143 | 51.976% ± 0.158 | 63.346% ± 0.152 | 36.654% ± 0.152 |
| Matchup-flex JIT, going second | 37.104% ± 0.153 | 60.856% ± 0.154 | 71.247% ± 0.143 | 28.753% ± 0.143 |
| No discard control, going second | 39.987% ± 0.155 | 66.854% ± 0.149 | 78.162% ± 0.131 | 21.838% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.977% ± 0.142 | 35.638% ± 0.151 | 64.362% ± 0.151 |
| Strict JIT, full Item lock, second | 10.488% ± 0.097 | 22.888% ± 0.133 | 30.049% ± 0.145 | 69.951% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.955% ± 0.121 | 34.447% ± 0.150 | 44.524% ± 0.157 | 55.476% ± 0.157 |
| Strict JIT, combined lock, second | 2.361% ± 0.048 | 11.368% ± 0.100 | 15.466% ± 0.114 | 84.534% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 10.163% ± 0.096 | 16.908% ± 0.119 | 83.092% ± 0.119 |
| Strict JIT, Supporter lock, second | 6.159% ± 0.076 | 14.670% ± 0.112 | 20.978% ± 0.129 | 79.022% ± 0.129 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% | 26.031% | 16.941% | 10.678% |
| Matchup-flex JIT, going first | 16.406% | 30.978% | 16.014% | 9.551% |
| No discard control, going first | 19.954% | 35.820% | 16.071% | 9.002% |
| Strict JIT, going second | 28.729% | 23.247% | 11.370% | 7.860% |
| Matchup-flex JIT, going second | 37.104% | 23.752% | 10.391% | 7.024% |
| No discard control, going second | 39.987% | 26.867% | 11.308% | 6.631% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.590% ± 0.123 | 44.352% ± 0.157 | 61.740% ± 0.154 | 38.260% ± 0.154 |
| Matchup-flex JIT, going first | 20.055% ± 0.127 | 45.331% ± 0.157 | 62.794% ± 0.153 | 37.206% ± 0.153 |
| No discard control, going first | 24.730% ± 0.136 | 57.809% ± 0.156 | 73.389% ± 0.140 | 26.611% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.527% ± 0.066 | 7.707% ± 0.084 | 13.293% ± 0.107 | 86.707% ± 0.107 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.672% ± 0.073 | 10.711% ± 0.098 | 89.289% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.697% ± 0.067 | 16.545% ± 0.118 | 27.527% ± 0.141 | 72.473% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.342% ± 0.158 | 61.184% ± 0.154 | 71.400% ± 0.143 | 28.600% ± 0.143 |
| Matchup-flex JIT, going second | 49.351% ± 0.158 | 63.969% ± 0.152 | 73.500% ± 0.140 | 26.500% ± 0.140 |
| No discard control, going second | 58.537% ± 0.156 | 72.764% ± 0.141 | 81.128% ± 0.124 | 18.872% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.600% ± 0.079 | 11.805% ± 0.102 | 17.497% ± 0.120 | 82.503% ± 0.120 |
| Strict JIT, full Item lock, second | 4.389% ± 0.065 | 9.130% ± 0.091 | 14.485% ± 0.111 | 85.515% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.639% ± 0.101 | 22.250% ± 0.132 | 31.365% ± 0.147 | 68.635% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.099% ± 0.033 | 5.943% ± 0.075 | 10.047% ± 0.095 | 89.953% ± 0.095 |
| Strict JIT, Supporter lock, second | 5.026% ± 0.069 | 9.533% ± 0.093 | 13.788% ± 0.109 | 86.212% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.590% | 25.762% | 17.388% | 10.324% |
| Matchup-flex JIT, going first | 20.055% | 25.276% | 17.463% | 10.531% |
| No discard control, going first | 24.730% | 33.079% | 15.580% | 7.763% |
| Strict JIT, going second | 46.342% | 14.842% | 10.216% | 6.407% |
| Matchup-flex JIT, going second | 49.351% | 14.618% | 9.531% | 6.035% |
| No discard control, going second | 58.537% | 14.227% | 8.364% | 4.782% |

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
| Secret Box use | 62.985% |
| Exploding Energy use | 77.826% |
| Steven use | 36.107% |
| Star Alchemy use | 47.937% |
| Secret Box attempts | 1.534 per game |
| Cost blocks | 0.042 per game |
| Missing route axis | 0.860 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.305 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.395 |
| Pineco/Forretress line | 0.473 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.031 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.516 |
| Discard zone | 0.282 |
| Stranded hand zone | 0.212 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `84580e6534e2b196033c72788b7288c0fc0cda4b68ab922b87a948af5248f89f`.

Comparison CSV SHA-256: `2e93b992c0756fbb084c823e125a6dd2768676fb29e202653c9d02e3fd99addd`.
