# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.808% | 18.464% | +6.656 pp | 38.046% | 43.958% | +5.912 pp | 54.987% | 61.563% | +6.576 pp |
| Strict JIT, going second | 28.853% | 45.803% | +16.950 pp | 52.174% | 60.739% | +8.565 pp | 63.490% | 71.166% | +7.676 pp |
| Matchup-flex JIT, going first | 16.174% | 19.739% | +3.565 pp | 47.248% | 45.433% | -1.815 pp | 63.136% | 63.151% | +0.015 pp |
| Matchup-flex JIT, going second | 36.753% | 48.624% | +11.871 pp | 60.649% | 63.781% | +3.132 pp | 71.109% | 73.583% | +2.474 pp |
| No discard control, going first | 19.933% | 24.669% | +4.736 pp | 55.574% | 58.065% | +2.491 pp | 71.683% | 73.801% | +2.118 pp |
| No discard control, going second | 39.845% | 58.351% | +18.506 pp | 66.994% | 72.786% | +5.792 pp | 78.455% | 81.267% | +2.812 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.808% ± 0.102 | 38.046% ± 0.154 | 54.987% ± 0.157 | 45.013% ± 0.157 |
| Matchup-flex JIT, going first | 16.174% ± 0.116 | 47.248% ± 0.158 | 63.136% ± 0.153 | 36.864% ± 0.153 |
| No discard control, going first | 19.933% ± 0.126 | 55.574% ± 0.157 | 71.683% ± 0.142 | 28.317% ± 0.142 |
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
| No discard control, going first | 19.933% | 35.641% | 16.109% | 9.042% |
| Strict JIT, going second | 28.853% | 23.321% | 11.316% | 7.861% |
| Matchup-flex JIT, going second | 36.753% | 23.896% | 10.460% | 7.047% |
| No discard control, going second | 39.845% | 27.149% | 11.461% | 6.548% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.464% ± 0.123 | 43.958% ± 0.157 | 61.563% ± 0.154 | 38.437% ± 0.154 |
| Matchup-flex JIT, going first | 19.739% ± 0.126 | 45.433% ± 0.157 | 63.151% ± 0.153 | 36.849% ± 0.153 |
| No discard control, going first | 24.669% ± 0.136 | 58.065% ± 0.156 | 73.801% ± 0.139 | 26.199% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.653% ± 0.067 | 7.988% ± 0.086 | 13.611% ± 0.108 | 86.389% ± 0.108 |
| Strict JIT, full Item lock, first | 2.784% ± 0.052 | 5.742% ± 0.074 | 11.124% ± 0.099 | 88.876% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.355% ± 0.071 | 19.229% ± 0.125 | 31.782% ± 0.147 | 68.218% ± 0.147 |
| Strict JIT, combined lock, first | 0.634% ± 0.025 | 1.863% ± 0.043 | 4.300% ± 0.064 | 95.700% ± 0.064 |
| Strict JIT, going second | 45.803% ± 0.158 | 60.739% ± 0.154 | 71.166% ± 0.143 | 28.834% ± 0.143 |
| Matchup-flex JIT, going second | 48.624% ± 0.158 | 63.781% ± 0.152 | 73.583% ± 0.139 | 26.417% ± 0.139 |
| No discard control, going second | 58.351% ± 0.156 | 72.786% ± 0.141 | 81.267% ± 0.123 | 18.733% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.704% ± 0.079 | 12.110% ± 0.103 | 17.907% ± 0.121 | 82.093% ± 0.121 |
| Strict JIT, full Item lock, second | 4.705% ± 0.067 | 9.544% ± 0.093 | 15.105% ± 0.113 | 84.895% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.268% ± 0.107 | 25.633% ± 0.138 | 36.203% ± 0.152 | 63.797% ± 0.152 |
| Strict JIT, combined lock, second | 1.374% ± 0.037 | 3.302% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.113% ± 0.033 | 5.968% ± 0.075 | 10.117% ± 0.095 | 89.883% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.966% ± 0.069 | 9.656% ± 0.093 | 13.972% ± 0.110 | 86.028% ± 0.110 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.464% | 25.494% | 17.605% | 10.692% |
| Matchup-flex JIT, going first | 19.739% | 25.694% | 17.718% | 10.528% |
| No discard control, going first | 24.669% | 33.396% | 15.736% | 7.879% |
| Strict JIT, going second | 45.803% | 14.936% | 10.427% | 6.715% |
| Matchup-flex JIT, going second | 48.624% | 15.157% | 9.802% | 6.274% |
| No discard control, going second | 58.351% | 14.435% | 8.481% | 4.993% |

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
| Secret Box use | 62.378% |
| Exploding Energy use | 82.833% |
| Steven use | 36.483% |
| Star Alchemy use | 46.148% |
| Secret Box attempts | 1.559 per game |
| Cost blocks | 0.051 per game |
| Missing route axis | 0.883 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.307 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.393 |
| Pineco/Forretress line | 0.496 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.031 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.170 |
| Known Prize zone | 0.530 |
| Discard zone | 0.300 |
| Stranded hand zone | 0.180 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `c691ceff4ce959b8e82b187d745a2d3c6d3d3f61ad1e6163835bccbb758444f3`.

Comparison CSV SHA-256: `8b7e001760ca7b476dacf97100cfba58b81ba8ecd9f04b7e07f6bb768ceeecb0`.
