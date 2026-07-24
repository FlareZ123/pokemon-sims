# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.941% | 18.685% | +6.744 pp | 38.456% | 44.394% | +5.938 pp | 55.321% | 61.602% | +6.281 pp |
| Strict JIT, going second | 29.016% | 46.057% | +17.041 pp | 52.427% | 60.904% | +8.477 pp | 63.622% | 71.017% | +7.395 pp |
| Matchup-flex JIT, going first | 16.406% | 19.917% | +3.511 pp | 47.384% | 45.441% | -1.943 pp | 63.398% | 62.928% | -0.470 pp |
| Matchup-flex JIT, going second | 37.104% | 49.140% | +12.036 pp | 60.856% | 64.096% | +3.240 pp | 71.247% | 73.646% | +2.399 pp |
| No discard control, going first | 19.954% | 24.568% | +4.614 pp | 55.774% | 57.494% | +1.720 pp | 71.845% | 73.118% | +1.273 pp |
| No discard control, going second | 39.987% | 58.592% | +18.605 pp | 66.854% | 72.511% | +5.657 pp | 78.162% | 80.971% | +2.809 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.941% ± 0.103 | 38.456% ± 0.154 | 55.321% ± 0.157 | 44.679% ± 0.157 |
| Matchup-flex JIT, going first | 16.406% ± 0.117 | 47.384% ± 0.158 | 63.398% ± 0.152 | 36.602% ± 0.152 |
| No discard control, going first | 19.954% ± 0.126 | 55.774% ± 0.157 | 71.845% ± 0.142 | 28.155% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.582% ± 0.066 | 10.187% ± 0.096 | 17.684% ± 0.121 | 82.316% ± 0.121 |
| Strict JIT, full Item lock, first | 2.820% ± 0.052 | 7.733% ± 0.084 | 15.056% ± 0.113 | 84.944% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.448% ± 0.065 | 25.895% ± 0.139 | 38.818% ± 0.154 | 61.182% ± 0.154 |
| Strict JIT, combined lock, first | 0.292% ± 0.017 | 3.275% ± 0.056 | 7.263% ± 0.082 | 92.737% ± 0.082 |
| Strict JIT, going second | 29.016% ± 0.144 | 52.427% ± 0.158 | 63.622% ± 0.152 | 36.378% ± 0.152 |
| Matchup-flex JIT, going second | 37.104% ± 0.153 | 60.856% ± 0.154 | 71.247% ± 0.143 | 28.753% ± 0.143 |
| No discard control, going second | 39.987% ± 0.155 | 66.854% ± 0.149 | 78.162% ± 0.131 | 21.838% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.977% ± 0.142 | 35.638% ± 0.151 | 64.362% ± 0.151 |
| Strict JIT, full Item lock, second | 10.488% ± 0.097 | 22.888% ± 0.133 | 30.049% ± 0.145 | 69.951% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.979% ± 0.121 | 34.485% ± 0.150 | 44.591% ± 0.157 | 55.409% ± 0.157 |
| Strict JIT, combined lock, second | 2.361% ± 0.048 | 11.368% ± 0.100 | 15.466% ± 0.114 | 84.534% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.257% ± 0.114 | 21.516% ± 0.130 | 78.484% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.085% ± 0.086 | 19.364% ± 0.125 | 25.257% ± 0.137 | 74.743% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.941% | 26.515% | 16.865% | 10.425% |
| Matchup-flex JIT, going first | 16.406% | 30.978% | 16.014% | 9.551% |
| No discard control, going first | 19.954% | 35.820% | 16.071% | 9.002% |
| Strict JIT, going second | 29.016% | 23.411% | 11.195% | 7.905% |
| Matchup-flex JIT, going second | 37.104% | 23.752% | 10.391% | 7.024% |
| No discard control, going second | 39.987% | 26.867% | 11.308% | 6.631% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.685% ± 0.123 | 44.394% ± 0.157 | 61.602% ± 0.154 | 38.398% ± 0.154 |
| Matchup-flex JIT, going first | 19.917% ± 0.126 | 45.441% ± 0.157 | 62.928% ± 0.153 | 37.072% ± 0.153 |
| No discard control, going first | 24.568% ± 0.136 | 57.494% ± 0.156 | 73.118% ± 0.140 | 26.882% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.530% ± 0.066 | 7.704% ± 0.084 | 13.285% ± 0.107 | 86.715% ± 0.107 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.672% ± 0.073 | 10.711% ± 0.098 | 89.289% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.636% ± 0.066 | 16.470% ± 0.117 | 27.547% ± 0.141 | 72.453% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.057% ± 0.158 | 60.904% ± 0.154 | 71.017% ± 0.143 | 28.983% ± 0.143 |
| Matchup-flex JIT, going second | 49.140% ± 0.158 | 64.096% ± 0.152 | 73.646% ± 0.139 | 26.354% ± 0.139 |
| No discard control, going second | 58.592% ± 0.156 | 72.511% ± 0.141 | 80.971% ± 0.124 | 19.029% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.600% ± 0.079 | 11.805% ± 0.102 | 17.497% ± 0.120 | 82.503% ± 0.120 |
| Strict JIT, full Item lock, second | 4.389% ± 0.065 | 9.130% ± 0.091 | 14.485% ± 0.111 | 85.515% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.731% ± 0.102 | 22.339% ± 0.132 | 31.509% ± 0.147 | 68.491% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.858% ± 0.043 | 5.771% ± 0.074 | 9.912% ± 0.094 | 90.088% ± 0.094 |
| Strict JIT, Supporter lock, second | 6.004% ± 0.075 | 9.976% ± 0.095 | 14.235% ± 0.110 | 85.765% ± 0.110 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.685% | 25.709% | 17.208% | 10.309% |
| Matchup-flex JIT, going first | 19.917% | 25.524% | 17.487% | 10.487% |
| No discard control, going first | 24.568% | 32.926% | 15.624% | 7.860% |
| Strict JIT, going second | 46.057% | 14.847% | 10.113% | 6.430% |
| Matchup-flex JIT, going second | 49.140% | 14.956% | 9.550% | 5.951% |
| No discard control, going second | 58.592% | 13.919% | 8.460% | 4.760% |

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
| Secret Box use | 63.036% |
| Exploding Energy use | 77.630% |
| Steven use | 36.101% |
| Star Alchemy use | 48.315% |
| Secret Box attempts | 1.548 per game |
| Cost blocks | 0.044 per game |
| Missing route axis | 0.872 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.277 per game |
| Steven banks | 0.304 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.403 |
| Pineco/Forretress line | 0.483 |
| VSTAR | 0.004 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.029 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.167 |
| Known Prize zone | 0.528 |
| Discard zone | 0.284 |
| Stranded hand zone | 0.217 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `42d067e768d660fdf2eece058daa13b409010b24650b5a7e8e27a99b8a413ee2`.

Comparison CSV SHA-256: `fac239fea0ff6c57d9d8f2129b6539ff06a871204a6726e2e6092386f802cfa6`.
