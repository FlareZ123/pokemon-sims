# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.932% | 18.693% | +6.761 pp | 38.446% | 44.310% | +5.864 pp | 55.293% | 61.498% | +6.205 pp |
| Strict JIT, going second | 29.073% | 46.073% | +17.000 pp | 52.467% | 60.978% | +8.511 pp | 63.781% | 71.134% | +7.353 pp |
| Matchup-flex JIT, going first | 16.371% | 19.871% | +3.500 pp | 47.333% | 45.400% | -1.933 pp | 63.334% | 62.922% | -0.412 pp |
| Matchup-flex JIT, going second | 37.005% | 49.087% | +12.082 pp | 60.702% | 64.017% | +3.315 pp | 71.103% | 73.512% | +2.409 pp |
| No discard control, going first | 19.972% | 24.598% | +4.626 pp | 55.863% | 57.557% | +1.694 pp | 71.922% | 73.158% | +1.236 pp |
| No discard control, going second | 39.956% | 58.674% | +18.718 pp | 66.931% | 72.638% | +5.707 pp | 78.204% | 81.109% | +2.905 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.932% ± 0.103 | 38.446% ± 0.154 | 55.293% ± 0.157 | 44.707% ± 0.157 |
| Matchup-flex JIT, going first | 16.371% ± 0.117 | 47.333% ± 0.158 | 63.334% ± 0.152 | 36.666% ± 0.152 |
| No discard control, going first | 19.972% ± 0.126 | 55.863% ± 0.157 | 71.922% ± 0.142 | 28.078% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.596% ± 0.066 | 10.188% ± 0.096 | 17.698% ± 0.121 | 82.302% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.332% ± 0.064 | 25.881% ± 0.139 | 38.859% ± 0.154 | 61.141% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.073% ± 0.144 | 52.467% ± 0.158 | 63.781% ± 0.152 | 36.219% ± 0.152 |
| Matchup-flex JIT, going second | 37.005% ± 0.153 | 60.702% ± 0.154 | 71.103% ± 0.143 | 28.897% ± 0.143 |
| No discard control, going second | 39.956% ± 0.155 | 66.931% ± 0.149 | 78.204% ± 0.131 | 21.796% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.081% ± 0.110 | 27.971% ± 0.142 | 35.630% ± 0.151 | 64.370% ± 0.151 |
| Strict JIT, full Item lock, second | 10.525% ± 0.097 | 22.920% ± 0.133 | 30.083% ± 0.145 | 69.917% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.942% ± 0.121 | 34.505% ± 0.150 | 44.704% ± 0.157 | 55.296% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.282% ± 0.114 | 21.547% ± 0.130 | 78.453% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.428% ± 0.125 | 25.336% ± 0.138 | 74.664% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.932% | 26.514% | 16.847% | 10.523% |
| Matchup-flex JIT, going first | 16.371% | 30.962% | 16.001% | 9.562% |
| No discard control, going first | 19.972% | 35.891% | 16.059% | 8.956% |
| Strict JIT, going second | 29.073% | 23.394% | 11.314% | 7.937% |
| Matchup-flex JIT, going second | 37.005% | 23.697% | 10.401% | 7.046% |
| No discard control, going second | 39.956% | 26.975% | 11.273% | 6.590% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.693% ± 0.123 | 44.310% ± 0.157 | 61.498% ± 0.154 | 38.502% ± 0.154 |
| Matchup-flex JIT, going first | 19.871% ± 0.126 | 45.400% ± 0.157 | 62.922% ± 0.153 | 37.078% ± 0.153 |
| No discard control, going first | 24.598% ± 0.136 | 57.557% ± 0.156 | 73.158% ± 0.140 | 26.842% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.530% ± 0.066 | 7.704% ± 0.084 | 13.285% ± 0.107 | 86.715% ± 0.107 |
| Strict JIT, full Item lock, first | 2.769% ± 0.052 | 5.672% ± 0.073 | 10.711% ± 0.098 | 89.289% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.611% ± 0.066 | 16.412% ± 0.117 | 27.429% ± 0.141 | 72.571% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.073% ± 0.158 | 60.978% ± 0.154 | 71.134% ± 0.143 | 28.866% ± 0.143 |
| Matchup-flex JIT, going second | 49.087% ± 0.158 | 64.017% ± 0.152 | 73.512% ± 0.140 | 26.488% ± 0.140 |
| No discard control, going second | 58.674% ± 0.156 | 72.638% ± 0.141 | 81.109% ± 0.124 | 18.891% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.600% ± 0.079 | 11.805% ± 0.102 | 17.497% ± 0.120 | 82.503% ± 0.120 |
| Strict JIT, full Item lock, second | 4.389% ± 0.065 | 9.130% ± 0.091 | 14.485% ± 0.111 | 85.515% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.770% ± 0.102 | 22.345% ± 0.132 | 31.569% ± 0.147 | 68.431% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.859% ± 0.043 | 5.782% ± 0.074 | 9.920% ± 0.095 | 90.080% ± 0.095 |
| Strict JIT, Supporter lock, second | 6.032% ± 0.075 | 9.972% ± 0.095 | 14.253% ± 0.111 | 85.747% ± 0.111 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.693% | 25.617% | 17.188% | 10.424% |
| Matchup-flex JIT, going first | 19.871% | 25.529% | 17.522% | 10.473% |
| No discard control, going first | 24.598% | 32.959% | 15.601% | 7.850% |
| Strict JIT, going second | 46.073% | 14.905% | 10.156% | 6.463% |
| Matchup-flex JIT, going second | 49.087% | 14.930% | 9.495% | 6.005% |
| No discard control, going second | 58.674% | 13.964% | 8.471% | 4.744% |

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
| Secret Box use | 63.048% |
| Exploding Energy use | 77.629% |
| Steven use | 36.118% |
| Star Alchemy use | 48.392% |
| Secret Box attempts | 1.544 per game |
| Cost blocks | 0.044 per game |
| Missing route axis | 0.868 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.305 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.402 |
| Pineco/Forretress line | 0.478 |
| VSTAR | 0.004 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.030 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.526 |
| Discard zone | 0.281 |
| Stranded hand zone | 0.215 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `1d24eafea0a180a78ecb4c010760ae5e0e6dcbf8b9cd71fdb6ab687651ea7c76`.

Comparison CSV SHA-256: `f174dd2d94b5cfaae11f5926662dd1888d6d69bd665bd079119c44cf636078f7`.
