# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.938% | 18.693% | +6.755 pp | 38.464% | 44.310% | +5.846 pp | 55.347% | 61.498% | +6.151 pp |
| Strict JIT, going second | 29.155% | 46.143% | +16.988 pp | 52.513% | 61.022% | +8.509 pp | 63.819% | 71.150% | +7.331 pp |
| Matchup-flex JIT, going first | 16.381% | 19.871% | +3.490 pp | 47.314% | 45.400% | -1.914 pp | 63.324% | 62.922% | -0.402 pp |
| Matchup-flex JIT, going second | 37.018% | 49.180% | +12.162 pp | 60.644% | 63.970% | +3.326 pp | 71.024% | 73.537% | +2.513 pp |
| No discard control, going first | 19.944% | 24.598% | +4.654 pp | 55.793% | 57.557% | +1.764 pp | 71.891% | 73.158% | +1.267 pp |
| No discard control, going second | 39.921% | 58.601% | +18.680 pp | 66.973% | 72.584% | +5.611 pp | 78.280% | 81.054% | +2.774 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.938% ± 0.103 | 38.464% ± 0.154 | 55.347% ± 0.157 | 44.653% ± 0.157 |
| Matchup-flex JIT, going first | 16.381% ± 0.117 | 47.314% ± 0.158 | 63.324% ± 0.152 | 36.676% ± 0.152 |
| No discard control, going first | 19.944% ± 0.126 | 55.793% ± 0.157 | 71.891% ± 0.142 | 28.109% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.339% ± 0.064 | 25.880% ± 0.139 | 38.878% ± 0.154 | 61.122% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.155% ± 0.144 | 52.513% ± 0.158 | 63.819% ± 0.152 | 36.181% ± 0.152 |
| Matchup-flex JIT, going second | 37.018% ± 0.153 | 60.644% ± 0.154 | 71.024% ± 0.143 | 28.976% ± 0.143 |
| No discard control, going second | 39.921% ± 0.155 | 66.973% ± 0.149 | 78.280% ± 0.130 | 21.720% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.941% ± 0.142 | 35.593% ± 0.151 | 64.407% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.934% ± 0.121 | 34.516% ± 0.150 | 44.731% ± 0.157 | 55.269% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.282% ± 0.114 | 21.547% ± 0.130 | 78.453% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.428% ± 0.125 | 25.336% ± 0.138 | 74.664% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.938% | 26.526% | 16.883% | 10.485% |
| Matchup-flex JIT, going first | 16.381% | 30.933% | 16.010% | 9.559% |
| No discard control, going first | 19.944% | 35.849% | 16.098% | 8.954% |
| Strict JIT, going second | 29.155% | 23.358% | 11.306% | 7.919% |
| Matchup-flex JIT, going second | 37.018% | 23.626% | 10.380% | 7.058% |
| No discard control, going second | 39.921% | 27.052% | 11.307% | 6.525% |

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
| Strict JIT, going second | 46.143% ± 0.158 | 61.022% ± 0.154 | 71.150% ± 0.143 | 28.850% ± 0.143 |
| Matchup-flex JIT, going second | 49.180% ± 0.158 | 63.970% ± 0.152 | 73.537% ± 0.139 | 26.463% ± 0.139 |
| No discard control, going second | 58.601% ± 0.156 | 72.584% ± 0.141 | 81.054% ± 0.124 | 18.946% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.602% ± 0.079 | 11.813% ± 0.102 | 17.517% ± 0.120 | 82.483% ± 0.120 |
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
| Strict JIT, going second | 46.143% | 14.879% | 10.128% | 6.452% |
| Matchup-flex JIT, going second | 49.180% | 14.790% | 9.567% | 6.010% |
| No discard control, going second | 58.601% | 13.983% | 8.470% | 4.739% |

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
| Secret Box use | 62.794% |
| Exploding Energy use | 77.588% |
| Steven use | 35.934% |
| Star Alchemy use | 48.285% |
| Secret Box attempts | 1.540 per game |
| Cost blocks | 0.043 per game |
| Missing route axis | 0.868 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.278 per game |
| Steven banks | 0.303 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.397 |
| Pineco/Forretress line | 0.482 |
| VSTAR | 0.004 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.030 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.167 |
| Known Prize zone | 0.525 |
| Discard zone | 0.283 |
| Stranded hand zone | 0.216 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `11a40133d3bb4cf197552becaa90f25c84d2c3ac5461b3be5197de0d285101fb`.

Comparison CSV SHA-256: `9cdd254d685a25a0f6f25d891fad56e018c9d73ade976e3a1e284594385f1926`.
