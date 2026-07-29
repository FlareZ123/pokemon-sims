# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.907% | 19.616% | +7.709 pp | 38.506% | 47.919% | +9.413 pp | 55.425% | 65.678% | +10.253 pp |
| Strict JIT, going second | 29.303% | 47.468% | +18.165 pp | 52.635% | 62.857% | +10.222 pp | 63.878% | 73.602% | +9.724 pp |
| Matchup-flex JIT, going first | 16.308% | 20.917% | +4.609 pp | 47.647% | 49.355% | +1.708 pp | 63.409% | 67.582% | +4.173 pp |
| Matchup-flex JIT, going second | 37.302% | 51.126% | +13.824 pp | 61.097% | 66.499% | +5.402 pp | 71.345% | 76.175% | +4.830 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.830% | 59.653% | +19.823 pp | 66.914% | 74.024% | +7.110 pp | 77.999% | 82.430% | +4.431 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.907% ± 0.102 | 38.506% ± 0.154 | 55.425% ± 0.157 | 44.575% ± 0.157 |
| Matchup-flex JIT, going first | 16.308% ± 0.117 | 47.647% ± 0.158 | 63.409% ± 0.152 | 36.591% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.574% ± 0.066 | 10.157% ± 0.096 | 17.686% ± 0.121 | 82.314% ± 0.121 |
| Strict JIT, full Item lock, first | 2.817% ± 0.052 | 7.746% ± 0.085 | 15.059% ± 0.113 | 84.941% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.383% ± 0.065 | 26.020% ± 0.139 | 39.115% ± 0.154 | 60.885% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.268% ± 0.082 | 92.732% ± 0.082 |
| Strict JIT, going second | 29.303% ± 0.144 | 52.635% ± 0.158 | 63.878% ± 0.152 | 36.122% ± 0.152 |
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
| Strict JIT, going first | 11.907% | 26.599% | 16.919% | 10.655% |
| Matchup-flex JIT, going first | 16.308% | 31.339% | 15.762% | 9.701% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.303% | 23.332% | 11.243% | 7.820% |
| Matchup-flex JIT, going second | 37.302% | 23.795% | 10.248% | 6.812% |
| No discard control, going second | 39.830% | 27.084% | 11.085% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% ± 0.126 | 47.919% ± 0.158 | 65.678% ± 0.150 | 34.322% ± 0.150 |
| Matchup-flex JIT, going first | 20.917% ± 0.129 | 49.355% ± 0.158 | 67.582% ± 0.148 | 32.418% ± 0.148 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.673% ± 0.067 | 8.706% ± 0.089 | 15.010% ± 0.113 | 84.990% ± 0.113 |
| Strict JIT, full Item lock, first | 2.861% ± 0.053 | 6.126% ± 0.076 | 11.399% ± 0.100 | 88.601% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.895% ± 0.068 | 18.237% ± 0.122 | 30.365% ± 0.145 | 69.635% ± 0.145 |
| Strict JIT, combined lock, first | 0.500% ± 0.022 | 1.451% ± 0.038 | 3.291% ± 0.056 | 96.709% ± 0.056 |
| Strict JIT, going second | 47.468% ± 0.158 | 62.857% ± 0.153 | 73.602% ± 0.139 | 26.398% ± 0.139 |
| Matchup-flex JIT, going second | 51.126% ± 0.158 | 66.499% ± 0.149 | 76.175% ± 0.135 | 23.825% ± 0.135 |
| No discard control, going second | 59.653% ± 0.155 | 74.024% ± 0.139 | 82.430% ± 0.120 | 17.570% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.919% ± 0.080 | 12.939% ± 0.106 | 19.139% ± 0.124 | 80.861% ± 0.124 |
| Strict JIT, full Item lock, second | 4.554% ± 0.066 | 9.502% ± 0.093 | 15.043% ± 0.113 | 84.957% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.324% ± 0.104 | 24.187% ± 0.135 | 33.887% ± 0.150 | 66.113% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.787% ± 0.052 | 4.753% ± 0.067 | 95.247% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.853% ± 0.043 | 6.899% ± 0.080 | 11.867% ± 0.102 | 88.133% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.303% ± 0.077 | 11.189% ± 0.100 | 16.301% ± 0.117 | 83.699% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% | 28.303% | 17.759% | 10.822% |
| Matchup-flex JIT, going first | 20.917% | 28.438% | 18.227% | 10.520% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.468% | 15.389% | 10.745% | 6.953% |
| Matchup-flex JIT, going second | 51.126% | 15.373% | 9.676% | 6.428% |
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

Simulator policy digest: `ef73df2645130740e2f2bd234d2be0dc167a29ffac43d78f20bccb6e2966857f`.

Comparison CSV SHA-256: `cbab52e29eed434c33726e27999f0f843a31f3f164fea07d5a7f01c410ce81f1`.
