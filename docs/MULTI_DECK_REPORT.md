# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.756% | 18.579% | +6.823 pp | 37.933% | 44.189% | +6.256 pp | 55.014% | 61.800% | +6.786 pp |
| Strict JIT, going second | 29.122% | 46.084% | +16.962 pp | 52.229% | 60.812% | +8.583 pp | 63.521% | 71.218% | +7.697 pp |
| Matchup-flex JIT, going first | 16.142% | 19.982% | +3.840 pp | 47.410% | 45.630% | -1.780 pp | 63.246% | 63.434% | +0.188 pp |
| Matchup-flex JIT, going second | 36.887% | 48.584% | +11.697 pp | 60.419% | 63.718% | +3.299 pp | 70.842% | 73.527% | +2.685 pp |
| No discard control, going first | 19.963% | 24.491% | +4.528 pp | 55.349% | 57.666% | +2.317 pp | 71.524% | 73.649% | +2.125 pp |
| No discard control, going second | 39.949% | 58.374% | +18.425 pp | 67.000% | 72.601% | +5.601 pp | 78.492% | 81.145% | +2.653 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.756% ± 0.102 | 37.933% ± 0.153 | 55.014% ± 0.157 | 44.986% ± 0.157 |
| Matchup-flex JIT, going first | 16.142% ± 0.116 | 47.410% ± 0.158 | 63.246% ± 0.152 | 36.754% ± 0.152 |
| No discard control, going first | 19.963% ± 0.126 | 55.349% ± 0.157 | 71.524% ± 0.143 | 28.476% ± 0.143 |
| Strict JIT, turn-two Item lock, first | 4.561% ± 0.066 | 10.161% ± 0.096 | 17.686% ± 0.121 | 82.314% ± 0.121 |
| Strict JIT, full Item lock, first | 2.821% ± 0.052 | 7.736% ± 0.084 | 15.064% ± 0.113 | 84.936% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.342% ± 0.064 | 25.565% ± 0.138 | 38.589% ± 0.154 | 61.411% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 29.122% ± 0.144 | 52.229% ± 0.158 | 63.521% ± 0.152 | 36.479% ± 0.152 |
| Matchup-flex JIT, going second | 36.887% ± 0.153 | 60.419% ± 0.155 | 70.842% ± 0.144 | 29.158% ± 0.144 |
| No discard control, going second | 39.949% ± 0.155 | 67.000% ± 0.149 | 78.492% ± 0.130 | 21.508% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.092% ± 0.110 | 27.903% ± 0.142 | 35.554% ± 0.151 | 64.446% ± 0.151 |
| Strict JIT, full Item lock, second | 10.493% ± 0.097 | 22.882% ± 0.133 | 30.032% ± 0.145 | 69.968% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.130% ± 0.122 | 34.547% ± 0.150 | 44.747% ± 0.157 | 55.253% ± 0.157 |
| Strict JIT, combined lock, second | 2.352% ± 0.048 | 11.342% ± 0.100 | 15.418% ± 0.114 | 84.582% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 10.236% ± 0.096 | 17.118% ± 0.119 | 82.882% ± 0.119 |
| Strict JIT, Supporter lock, second | 6.118% ± 0.076 | 14.508% ± 0.111 | 20.918% ± 0.129 | 79.082% ± 0.129 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.756% | 26.177% | 17.081% | 10.801% |
| Matchup-flex JIT, going first | 16.142% | 31.268% | 15.836% | 9.593% |
| No discard control, going first | 19.963% | 35.386% | 16.175% | 9.033% |
| Strict JIT, going second | 29.122% | 23.107% | 11.292% | 7.812% |
| Matchup-flex JIT, going second | 36.887% | 23.532% | 10.423% | 7.148% |
| No discard control, going second | 39.949% | 27.051% | 11.492% | 6.484% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.579% ± 0.123 | 44.189% ± 0.157 | 61.800% ± 0.154 | 38.200% ± 0.154 |
| Matchup-flex JIT, going first | 19.982% ± 0.126 | 45.630% ± 0.158 | 63.434% ± 0.152 | 36.566% ± 0.152 |
| No discard control, going first | 24.491% ± 0.136 | 57.666% ± 0.156 | 73.649% ± 0.139 | 26.351% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.705% ± 0.067 | 7.986% ± 0.086 | 13.602% ± 0.108 | 86.398% ± 0.108 |
| Strict JIT, full Item lock, first | 2.784% ± 0.052 | 5.742% ± 0.074 | 11.124% ± 0.099 | 88.876% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.370% ± 0.071 | 19.132% ± 0.124 | 32.077% ± 0.148 | 67.923% ± 0.148 |
| Strict JIT, combined lock, first | 0.634% ± 0.025 | 1.863% ± 0.043 | 4.300% ± 0.064 | 95.700% ± 0.064 |
| Strict JIT, going second | 46.084% ± 0.158 | 60.812% ± 0.154 | 71.218% ± 0.143 | 28.782% ± 0.143 |
| Matchup-flex JIT, going second | 48.584% ± 0.158 | 63.718% ± 0.152 | 73.527% ± 0.140 | 26.473% ± 0.140 |
| No discard control, going second | 58.374% ± 0.156 | 72.601% ± 0.141 | 81.145% ± 0.124 | 18.855% ± 0.124 |
| Strict JIT, turn-two Item lock, second | 6.710% ± 0.079 | 12.105% ± 0.103 | 17.884% ± 0.121 | 82.116% ± 0.121 |
| Strict JIT, full Item lock, second | 4.705% ± 0.067 | 9.544% ± 0.093 | 15.105% ± 0.113 | 84.895% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.147% ± 0.107 | 25.623% ± 0.138 | 36.271% ± 0.152 | 63.729% ± 0.152 |
| Strict JIT, combined lock, second | 1.374% ± 0.037 | 3.302% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.124% ± 0.033 | 5.932% ± 0.075 | 10.035% ± 0.095 | 89.965% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.965% ± 0.069 | 9.664% ± 0.093 | 13.837% ± 0.109 | 86.163% ± 0.109 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.579% | 25.610% | 17.611% | 10.560% |
| Matchup-flex JIT, going first | 19.982% | 25.648% | 17.804% | 10.453% |
| No discard control, going first | 24.491% | 33.175% | 15.983% | 7.971% |
| Strict JIT, going second | 46.084% | 14.728% | 10.406% | 6.770% |
| Matchup-flex JIT, going second | 48.584% | 15.134% | 9.809% | 6.388% |
| No discard control, going second | 58.374% | 14.227% | 8.544% | 5.083% |

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
| Secret Box use | 62.554% |
| Exploding Energy use | 83.009% |
| Steven use | 36.187% |
| Star Alchemy use | 46.536% |
| Secret Box attempts | 1.567 per game |
| Cost blocks | 0.050 per game |
| Missing route axis | 0.889 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.305 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.500 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.029 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.170 |
| Known Prize zone | 0.531 |
| Discard zone | 0.305 |
| Stranded hand zone | 0.183 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `675364764ae80a7427d135a8bfdd5caa366894df262e2ce2df3672dd2091e0a3`.

Comparison CSV SHA-256: `586655436158181c8da0d24e720f69ff101f6c0bd19547234df6168437d32a85`.
