# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.977% | 18.707% | +6.730 pp | 38.745% | 44.488% | +5.743 pp | 55.512% | 61.976% | +6.464 pp |
| Strict JIT, going second | 29.402% | 46.237% | +16.835 pp | 52.755% | 61.141% | +8.386 pp | 63.927% | 71.527% | +7.600 pp |
| Matchup-flex JIT, going first | 16.295% | 19.998% | +3.703 pp | 47.322% | 45.587% | -1.735 pp | 63.313% | 63.328% | +0.015 pp |
| Matchup-flex JIT, going second | 37.322% | 49.304% | +11.982 pp | 60.735% | 64.320% | +3.585 pp | 71.007% | 73.947% | +2.940 pp |
| No discard control, going first | 19.986% | 24.620% | +4.634 pp | 55.868% | 57.796% | +1.928 pp | 71.928% | 73.581% | +1.653 pp |
| No discard control, going second | 39.926% | 58.606% | +18.680 pp | 66.904% | 72.762% | +5.858 pp | 78.044% | 81.340% | +3.296 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.977% ± 0.103 | 38.745% ± 0.154 | 55.512% ± 0.157 | 44.488% ± 0.157 |
| Matchup-flex JIT, going first | 16.295% ± 0.117 | 47.322% ± 0.158 | 63.313% ± 0.152 | 36.687% ± 0.152 |
| No discard control, going first | 19.986% ± 0.126 | 55.868% ± 0.157 | 71.928% ± 0.142 | 28.072% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.420% ± 0.065 | 25.911% ± 0.139 | 38.953% ± 0.154 | 61.047% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.402% ± 0.144 | 52.755% ± 0.158 | 63.927% ± 0.152 | 36.073% ± 0.152 |
| Matchup-flex JIT, going second | 37.322% ± 0.153 | 60.735% ± 0.154 | 71.007% ± 0.143 | 28.993% ± 0.143 |
| No discard control, going second | 39.926% ± 0.155 | 66.904% ± 0.149 | 78.044% ± 0.131 | 21.956% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.941% ± 0.142 | 35.593% ± 0.151 | 64.407% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.076% ± 0.122 | 34.639% ± 0.150 | 44.766% ± 0.157 | 55.234% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.289% ± 0.114 | 21.545% ± 0.130 | 78.455% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.122% ± 0.086 | 19.457% ± 0.125 | 25.347% ± 0.138 | 74.653% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.977% | 26.768% | 16.767% | 10.533% |
| Matchup-flex JIT, going first | 16.295% | 31.027% | 15.991% | 9.675% |
| No discard control, going first | 19.986% | 35.882% | 16.060% | 8.954% |
| Strict JIT, going second | 29.402% | 23.353% | 11.172% | 7.902% |
| Matchup-flex JIT, going second | 37.322% | 23.413% | 10.272% | 6.997% |
| No discard control, going second | 39.926% | 26.978% | 11.140% | 6.621% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.707% ± 0.123 | 44.488% ± 0.157 | 61.976% ± 0.154 | 38.024% ± 0.154 |
| Matchup-flex JIT, going first | 19.998% ± 0.126 | 45.587% ± 0.157 | 63.328% ± 0.152 | 36.672% ± 0.152 |
| No discard control, going first | 24.620% ± 0.136 | 57.796% ± 0.156 | 73.581% ± 0.139 | 26.419% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.658% ± 0.067 | 16.493% ± 0.117 | 27.582% ± 0.141 | 72.418% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.237% ± 0.158 | 61.141% ± 0.154 | 71.527% ± 0.143 | 28.473% ± 0.143 |
| Matchup-flex JIT, going second | 49.304% ± 0.158 | 64.320% ± 0.151 | 73.947% ± 0.139 | 26.053% ± 0.139 |
| No discard control, going second | 58.606% ± 0.156 | 72.762% ± 0.141 | 81.340% ± 0.123 | 18.660% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.604% ± 0.079 | 11.845% ± 0.102 | 17.560% ± 0.120 | 82.440% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.743% ± 0.102 | 22.357% ± 0.132 | 31.611% ± 0.147 | 68.389% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 5.803% ± 0.074 | 9.958% ± 0.095 | 90.042% ± 0.095 |
| Strict JIT, Supporter lock, second | 6.022% ± 0.075 | 9.967% ± 0.095 | 14.277% ± 0.111 | 85.723% ± 0.111 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.707% | 25.781% | 17.488% | 10.540% |
| Matchup-flex JIT, going first | 19.998% | 25.589% | 17.741% | 10.451% |
| No discard control, going first | 24.620% | 33.176% | 15.785% | 7.983% |
| Strict JIT, going second | 46.237% | 14.904% | 10.386% | 6.479% |
| Matchup-flex JIT, going second | 49.304% | 15.016% | 9.627% | 6.143% |
| No discard control, going second | 58.606% | 14.156% | 8.578% | 4.868% |

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
| Secret Box use | 62.728% |
| Exploding Energy use | 78.146% |
| Steven use | 35.889% |
| Star Alchemy use | 48.120% |
| Secret Box attempts | 1.545 per game |
| Cost blocks | 0.045 per game |
| Missing route axis | 0.872 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.303 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.403 |
| Pineco/Forretress line | 0.481 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.029 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.521 |
| Discard zone | 0.288 |
| Stranded hand zone | 0.214 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `7703fd09f1247f7d5d5f1a0017a3f60146ef01553d37385e480d43efb824d6e8`.

Comparison CSV SHA-256: `da651a425217f71a95ed50bb4a015196e9b29d8c293c8bf0992afafc8f59da9e`.
