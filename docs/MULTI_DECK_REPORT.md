# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.937% | 18.764% | +6.827 pp | 38.460% | 44.551% | +6.091 pp | 55.345% | 61.974% | +6.629 pp |
| Strict JIT, going second | 29.141% | 45.987% | +16.846 pp | 52.492% | 61.068% | +8.576 pp | 63.816% | 71.407% | +7.591 pp |
| Matchup-flex JIT, going first | 16.381% | 20.032% | +3.651 pp | 47.314% | 45.833% | -1.481 pp | 63.324% | 63.432% | +0.108 pp |
| Matchup-flex JIT, going second | 37.032% | 49.302% | +12.270 pp | 60.714% | 64.437% | +3.723 pp | 71.119% | 73.979% | +2.860 pp |
| No discard control, going first | 19.981% | 24.775% | +4.794 pp | 55.805% | 57.911% | +2.106 pp | 71.927% | 73.651% | +1.724 pp |
| No discard control, going second | 39.934% | 58.703% | +18.769 pp | 66.947% | 72.888% | +5.941 pp | 78.252% | 81.377% | +3.125 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.937% ± 0.103 | 38.460% ± 0.154 | 55.345% ± 0.157 | 44.655% ± 0.157 |
| Matchup-flex JIT, going first | 16.381% ± 0.117 | 47.314% ± 0.158 | 63.324% ± 0.152 | 36.676% ± 0.152 |
| No discard control, going first | 19.981% ± 0.126 | 55.805% ± 0.157 | 71.927% ± 0.142 | 28.073% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.339% ± 0.064 | 25.880% ± 0.139 | 38.878% ± 0.154 | 61.122% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.141% ± 0.144 | 52.492% ± 0.158 | 63.816% ± 0.152 | 36.184% ± 0.152 |
| Matchup-flex JIT, going second | 37.032% ± 0.153 | 60.714% ± 0.154 | 71.119% ± 0.143 | 28.881% ± 0.143 |
| No discard control, going second | 39.934% ± 0.155 | 66.947% ± 0.149 | 78.252% ± 0.130 | 21.748% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.081% ± 0.110 | 27.971% ± 0.142 | 35.630% ± 0.151 | 64.370% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 17.966% ± 0.121 | 34.537% ± 0.150 | 44.760% ± 0.157 | 55.240% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.282% ± 0.114 | 21.547% ± 0.130 | 78.453% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.428% ± 0.125 | 25.336% ± 0.138 | 74.664% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.937% | 26.523% | 16.885% | 10.483% |
| Matchup-flex JIT, going first | 16.381% | 30.933% | 16.010% | 9.559% |
| No discard control, going first | 19.981% | 35.824% | 16.122% | 8.952% |
| Strict JIT, going second | 29.141% | 23.351% | 11.324% | 7.945% |
| Matchup-flex JIT, going second | 37.032% | 23.682% | 10.405% | 7.048% |
| No discard control, going second | 39.934% | 27.013% | 11.305% | 6.584% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.764% ± 0.123 | 44.551% ± 0.157 | 61.974% ± 0.154 | 38.026% ± 0.154 |
| Matchup-flex JIT, going first | 20.032% ± 0.127 | 45.833% ± 0.158 | 63.432% ± 0.152 | 36.568% ± 0.152 |
| No discard control, going first | 24.775% ± 0.137 | 57.911% ± 0.156 | 73.651% ± 0.139 | 26.349% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.507% ± 0.066 | 7.689% ± 0.084 | 13.256% ± 0.107 | 86.744% ± 0.107 |
| Strict JIT, full Item lock, first | 2.774% ± 0.052 | 5.661% ± 0.073 | 10.720% ± 0.098 | 89.280% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.643% ± 0.067 | 16.514% ± 0.117 | 27.584% ± 0.141 | 72.416% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 45.987% ± 0.158 | 61.068% ± 0.154 | 71.407% ± 0.143 | 28.593% ± 0.143 |
| Matchup-flex JIT, going second | 49.302% ± 0.158 | 64.437% ± 0.151 | 73.979% ± 0.139 | 26.021% ± 0.139 |
| No discard control, going second | 58.703% ± 0.156 | 72.888% ± 0.141 | 81.377% ± 0.123 | 18.623% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.611% ± 0.079 | 11.847% ± 0.102 | 17.550% ± 0.120 | 82.450% ± 0.120 |
| Strict JIT, full Item lock, second | 4.401% ± 0.065 | 9.141% ± 0.091 | 14.508% ± 0.111 | 85.492% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.758% ± 0.102 | 22.343% ± 0.132 | 31.611% ± 0.147 | 68.389% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.858% ± 0.043 | 5.803% ± 0.074 | 9.955% ± 0.095 | 90.045% ± 0.095 |
| Strict JIT, Supporter lock, second | 6.032% ± 0.075 | 9.963% ± 0.095 | 14.271% ± 0.111 | 85.729% ± 0.111 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.764% | 25.787% | 17.423% | 10.575% |
| Matchup-flex JIT, going first | 20.032% | 25.801% | 17.599% | 10.391% |
| No discard control, going first | 24.775% | 33.136% | 15.740% | 7.988% |
| Strict JIT, going second | 45.987% | 15.081% | 10.339% | 6.584% |
| Matchup-flex JIT, going second | 49.302% | 15.135% | 9.542% | 6.137% |
| No discard control, going second | 58.703% | 14.185% | 8.489% | 4.877% |

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
| Secret Box use | 62.881% |
| Exploding Energy use | 78.033% |
| Steven use | 36.194% |
| Star Alchemy use | 48.315% |
| Secret Box attempts | 1.534 per game |
| Cost blocks | 0.045 per game |
| Missing route axis | 0.859 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.272 per game |
| Steven banks | 0.306 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.403 |
| Pineco/Forretress line | 0.471 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.028 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.165 |
| Known Prize zone | 0.516 |
| Discard zone | 0.281 |
| Stranded hand zone | 0.210 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `58245f9bb685017b5c3ee7f9d1d094e914bd0d11e8a84717276eb3ed2523b7d3`.

Comparison CSV SHA-256: `dd93e685b8f6850fca88be01fd7117b2442fdccdcdeac4d6ba5788693e400af3`.
