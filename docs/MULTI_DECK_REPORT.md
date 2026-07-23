# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.969% | 18.553% | +6.584 pp | 38.000% | 44.298% | +6.298 pp | 54.941% | 61.695% | +6.754 pp |
| Strict JIT, going second | 28.729% | 46.203% | +17.474 pp | 51.976% | 60.934% | +8.958 pp | 63.346% | 71.117% | +7.771 pp |
| Matchup-flex JIT, going first | 16.406% | 19.997% | +3.591 pp | 47.384% | 45.402% | -1.982 pp | 63.398% | 62.843% | -0.555 pp |
| Matchup-flex JIT, going second | 37.104% | 49.194% | +12.090 pp | 60.856% | 63.914% | +3.058 pp | 71.247% | 73.392% | +2.145 pp |
| No discard control, going first | 19.954% | 24.856% | +4.902 pp | 55.774% | 57.970% | +2.196 pp | 71.845% | 73.485% | +1.640 pp |
| No discard control, going second | 39.987% | 58.519% | +18.532 pp | 66.854% | 72.555% | +5.701 pp | 78.162% | 80.950% | +2.788 pp |

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
| Strict JIT, going second | 28.729% ± 0.143 | 51.976% ± 0.158 | 63.346% ± 0.152 | 36.654% ± 0.152 |
| Matchup-flex JIT, going second | 37.104% ± 0.153 | 60.856% ± 0.154 | 71.247% ± 0.143 | 28.753% ± 0.143 |
| No discard control, going second | 39.987% ± 0.155 | 66.854% ± 0.149 | 78.162% ± 0.131 | 21.838% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.976% ± 0.142 | 35.637% ± 0.151 | 64.363% ± 0.151 |
| Strict JIT, full Item lock, second | 10.497% ± 0.097 | 22.886% ± 0.133 | 30.046% ± 0.145 | 69.954% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.968% ± 0.121 | 34.458% ± 0.150 | 44.600% ± 0.157 | 55.400% ± 0.157 |
| Strict JIT, combined lock, second | 2.352% ± 0.048 | 11.342% ± 0.100 | 15.418% ± 0.114 | 84.582% ± 0.114 |
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
| Strict JIT, going first | 18.553% ± 0.123 | 44.298% ± 0.157 | 61.695% ± 0.154 | 38.305% ± 0.154 |
| Matchup-flex JIT, going first | 19.997% ± 0.126 | 45.402% ± 0.157 | 62.843% ± 0.153 | 37.157% ± 0.153 |
| No discard control, going first | 24.856% ± 0.137 | 57.970% ± 0.156 | 73.485% ± 0.140 | 26.515% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.527% ± 0.066 | 7.707% ± 0.084 | 13.293% ± 0.107 | 86.707% ± 0.107 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.672% ± 0.073 | 10.711% ± 0.098 | 89.289% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.674% ± 0.067 | 16.599% ± 0.118 | 27.654% ± 0.141 | 72.346% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.203% ± 0.158 | 60.934% ± 0.154 | 71.117% ± 0.143 | 28.883% ± 0.143 |
| Matchup-flex JIT, going second | 49.194% ± 0.158 | 63.914% ± 0.152 | 73.392% ± 0.140 | 26.608% ± 0.140 |
| No discard control, going second | 58.519% ± 0.156 | 72.555% ± 0.141 | 80.950% ± 0.124 | 19.050% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.600% ± 0.079 | 11.805% ± 0.102 | 17.497% ± 0.120 | 82.503% ± 0.120 |
| Strict JIT, full Item lock, second | 4.389% ± 0.065 | 9.130% ± 0.091 | 14.485% ± 0.111 | 85.515% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.684% ± 0.102 | 22.299% ± 0.132 | 31.406% ± 0.147 | 68.594% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.098% ± 0.033 | 5.947% ± 0.075 | 10.047% ± 0.095 | 89.953% ± 0.095 |
| Strict JIT, Supporter lock, second | 5.018% ± 0.069 | 9.530% ± 0.093 | 13.788% ± 0.109 | 86.212% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.553% | 25.745% | 17.397% | 10.478% |
| Matchup-flex JIT, going first | 19.997% | 25.405% | 17.441% | 10.517% |
| No discard control, going first | 24.856% | 33.114% | 15.515% | 7.678% |
| Strict JIT, going second | 46.203% | 14.731% | 10.183% | 6.428% |
| Matchup-flex JIT, going second | 49.194% | 14.720% | 9.478% | 6.045% |
| No discard control, going second | 58.519% | 14.036% | 8.395% | 4.782% |

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
| Secret Box use | 62.780% |
| Exploding Energy use | 77.685% |
| Steven use | 35.961% |
| Star Alchemy use | 47.613% |
| Secret Box attempts | 1.539 per game |
| Cost blocks | 0.045 per game |
| Missing route axis | 0.865 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.304 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.473 |
| VSTAR | 0.004 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.030 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.507 |
| Discard zone | 0.281 |
| Stranded hand zone | 0.209 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `0a35948db0c2b57050af8b9addf07de58bec351fc5b19542f03442034562d6e5`.

Comparison CSV SHA-256: `caf3b34fb2e6fbeda3e5b893707d0341392390f8c95f0dc05a4e5698071b5d44`.
