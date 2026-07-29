# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.923% | 19.470% | +7.547 pp | 38.563% | 47.151% | +8.588 pp | 55.449% | 65.231% | +9.782 pp |
| Strict JIT, going second | 29.299% | 47.410% | +18.111 pp | 52.629% | 62.605% | +9.976 pp | 63.869% | 73.301% | +9.432 pp |
| Matchup-flex JIT, going first | 16.308% | 20.864% | +4.556 pp | 47.647% | 48.435% | +0.788 pp | 63.409% | 66.753% | +3.344 pp |
| Matchup-flex JIT, going second | 37.302% | 50.686% | +13.384 pp | 61.097% | 65.824% | +4.727 pp | 71.345% | 75.507% | +4.162 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.830% | 59.653% | +19.823 pp | 66.914% | 74.024% | +7.110 pp | 77.999% | 82.430% | +4.431 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.923% ± 0.102 | 38.563% ± 0.154 | 55.449% ± 0.157 | 44.551% ± 0.157 |
| Matchup-flex JIT, going first | 16.308% ± 0.117 | 47.647% ± 0.158 | 63.409% ± 0.152 | 36.591% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.574% ± 0.066 | 10.157% ± 0.096 | 17.686% ± 0.121 | 82.314% ± 0.121 |
| Strict JIT, full Item lock, first | 2.817% ± 0.052 | 7.746% ± 0.085 | 15.059% ± 0.113 | 84.941% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.383% ± 0.065 | 26.020% ± 0.139 | 39.115% ± 0.154 | 60.885% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.268% ± 0.082 | 92.732% ± 0.082 |
| Strict JIT, going second | 29.299% ± 0.144 | 52.629% ± 0.158 | 63.869% ± 0.152 | 36.131% ± 0.152 |
| Matchup-flex JIT, going second | 37.302% ± 0.153 | 61.097% ± 0.154 | 71.345% ± 0.143 | 28.655% ± 0.143 |
| No discard control, going second | 39.830% ± 0.155 | 66.914% ± 0.149 | 77.999% ± 0.131 | 22.001% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.068% ± 0.110 | 27.943% ± 0.142 | 35.549% ± 0.151 | 64.451% ± 0.151 |
| Strict JIT, full Item lock, second | 10.558% ± 0.097 | 22.930% ± 0.133 | 30.109% ± 0.145 | 69.891% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.012% ± 0.122 | 34.650% ± 0.150 | 44.938% ± 0.157 | 55.062% ± 0.157 |
| Strict JIT, combined lock, second | 2.365% ± 0.048 | 11.404% ± 0.101 | 15.519% ± 0.115 | 84.481% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.327% ± 0.114 | 21.605% ± 0.130 | 78.395% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.192% ± 0.087 | 19.690% ± 0.126 | 25.501% ± 0.138 | 74.499% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.923% | 26.640% | 16.886% | 10.638% |
| Matchup-flex JIT, going first | 16.308% | 31.339% | 15.762% | 9.701% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.299% | 23.330% | 11.240% | 7.817% |
| Matchup-flex JIT, going second | 37.302% | 23.795% | 10.248% | 6.812% |
| No discard control, going second | 39.830% | 27.084% | 11.085% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.470% ± 0.125 | 47.151% ± 0.158 | 65.231% ± 0.151 | 34.769% ± 0.151 |
| Matchup-flex JIT, going first | 20.864% ± 0.128 | 48.435% ± 0.158 | 66.753% ± 0.149 | 33.247% ± 0.149 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.504% ± 0.066 | 7.700% ± 0.084 | 13.274% ± 0.107 | 86.726% ± 0.107 |
| Strict JIT, full Item lock, first | 2.776% ± 0.052 | 5.693% ± 0.073 | 10.734% ± 0.098 | 89.266% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.822% ± 0.068 | 17.800% ± 0.121 | 29.938% ± 0.145 | 70.062% ± 0.145 |
| Strict JIT, combined lock, first | 0.478% ± 0.022 | 1.344% ± 0.036 | 3.102% ± 0.055 | 96.898% ± 0.055 |
| Strict JIT, going second | 47.410% ± 0.158 | 62.605% ± 0.153 | 73.301% ± 0.140 | 26.699% ± 0.140 |
| Matchup-flex JIT, going second | 50.686% ± 0.158 | 65.824% ± 0.150 | 75.507% ± 0.136 | 24.493% ± 0.136 |
| No discard control, going second | 59.653% ± 0.155 | 74.024% ± 0.139 | 82.430% ± 0.120 | 17.570% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.610% ± 0.079 | 11.855% ± 0.102 | 17.576% ± 0.120 | 82.424% ± 0.120 |
| Strict JIT, full Item lock, second | 4.394% ± 0.065 | 9.124% ± 0.091 | 14.468% ± 0.111 | 85.532% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 12.180% ± 0.103 | 23.883% ± 0.135 | 33.510% ± 0.149 | 66.490% ± 0.149 |
| Strict JIT, combined lock, second | 1.163% ± 0.034 | 2.605% ± 0.050 | 4.447% ± 0.065 | 95.553% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.859% ± 0.043 | 5.978% ± 0.075 | 10.524% ± 0.097 | 89.476% ± 0.097 |
| Strict JIT, Supporter lock, second | 6.051% ± 0.075 | 10.112% ± 0.095 | 14.859% ± 0.112 | 85.141% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.470% | 27.681% | 18.080% | 10.950% |
| Matchup-flex JIT, going first | 20.864% | 27.571% | 18.318% | 11.026% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.410% | 15.195% | 10.696% | 7.218% |
| Matchup-flex JIT, going second | 50.686% | 15.138% | 9.683% | 6.857% |
| No discard control, going second | 59.653% | 14.371% | 8.406% | 4.598% |

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
| Secret Box use | 64.384% |
| Exploding Energy use | 78.624% |
| Steven use | 35.752% |
| Star Alchemy use | 48.074% |
| Secret Box attempts | 1.471 per game |
| Cost blocks | 0.048 per game |
| Missing route axis | 0.778 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.397 |
| Pineco/Forretress line | 0.431 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.155 |
| Known Prize zone | 0.477 |
| Discard zone | 0.238 |
| Stranded hand zone | 0.191 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `e77c0b765ba8e4612df977a21d4a8103d5f9ff12b94cb5cf6163825ebab7a8a6`.

Comparison CSV SHA-256: `f00c9310a79fa8df77c2d0523b3793d46c8545220db261f8eb46a9256ec41dab`.
