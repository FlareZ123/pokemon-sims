# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.885% | 19.616% | +7.731 pp | 38.731% | 47.906% | +9.175 pp | 55.812% | 65.633% | +9.821 pp |
| Strict JIT, going second | 29.340% | 47.473% | +18.133 pp | 52.609% | 62.843% | +10.234 pp | 63.963% | 73.603% | +9.640 pp |
| Matchup-flex JIT, going first | 16.382% | 21.020% | +4.638 pp | 47.638% | 49.173% | +1.535 pp | 63.393% | 67.420% | +4.027 pp |
| Matchup-flex JIT, going second | 37.288% | 51.063% | +13.775 pp | 61.121% | 66.446% | +5.325 pp | 71.400% | 76.141% | +4.741 pp |
| No discard control, going first | 20.090% | 25.361% | +5.271 pp | 55.965% | 60.065% | +4.100 pp | 72.284% | 75.670% | +3.386 pp |
| No discard control, going second | 39.873% | 59.653% | +19.780 pp | 67.048% | 74.024% | +6.976 pp | 78.156% | 82.430% | +4.274 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.885% ± 0.102 | 38.731% ± 0.154 | 55.812% ± 0.157 | 44.188% ± 0.157 |
| Matchup-flex JIT, going first | 16.382% ± 0.117 | 47.638% ± 0.158 | 63.393% ± 0.152 | 36.607% ± 0.152 |
| No discard control, going first | 20.090% ± 0.127 | 55.965% ± 0.157 | 72.284% ± 0.142 | 27.716% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.579% ± 0.066 | 10.127% ± 0.095 | 17.708% ± 0.121 | 82.292% ± 0.121 |
| Strict JIT, full Item lock, first | 2.789% ± 0.052 | 7.658% ± 0.084 | 15.045% ± 0.113 | 84.955% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.372% ± 0.065 | 26.093% ± 0.139 | 39.266% ± 0.154 | 60.734% ± 0.154 |
| Strict JIT, combined lock, first | 0.278% ± 0.017 | 3.231% ± 0.056 | 7.225% ± 0.082 | 92.775% ± 0.082 |
| Strict JIT, going second | 29.340% ± 0.144 | 52.609% ± 0.158 | 63.963% ± 0.152 | 36.037% ± 0.152 |
| Matchup-flex JIT, going second | 37.288% ± 0.153 | 61.121% ± 0.154 | 71.400% ± 0.143 | 28.600% ± 0.143 |
| No discard control, going second | 39.873% ± 0.155 | 67.048% ± 0.149 | 78.156% ± 0.131 | 21.844% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.027% ± 0.110 | 27.939% ± 0.142 | 35.743% ± 0.152 | 64.257% ± 0.152 |
| Strict JIT, full Item lock, second | 10.460% ± 0.097 | 22.819% ± 0.133 | 29.995% ± 0.145 | 70.005% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.144% ± 0.122 | 34.767% ± 0.151 | 45.178% ± 0.157 | 54.822% ± 0.157 |
| Strict JIT, combined lock, second | 2.330% ± 0.048 | 11.453% ± 0.101 | 15.595% ± 0.115 | 84.405% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.327% ± 0.114 | 21.605% ± 0.130 | 78.395% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.192% ± 0.087 | 19.690% ± 0.126 | 25.501% ± 0.138 | 74.499% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.885% | 26.846% | 17.081% | 10.627% |
| Matchup-flex JIT, going first | 16.382% | 31.256% | 15.755% | 9.694% |
| No discard control, going first | 20.090% | 35.875% | 16.319% | 8.988% |
| Strict JIT, going second | 29.340% | 23.269% | 11.354% | 7.875% |
| Matchup-flex JIT, going second | 37.288% | 23.833% | 10.279% | 6.847% |
| No discard control, going second | 39.873% | 27.175% | 11.108% | 6.620% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% ± 0.126 | 47.906% ± 0.158 | 65.633% ± 0.150 | 34.367% ± 0.150 |
| Matchup-flex JIT, going first | 21.020% ± 0.129 | 49.173% ± 0.158 | 67.420% ± 0.148 | 32.580% ± 0.148 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.697% ± 0.067 | 8.691% ± 0.089 | 14.846% ± 0.112 | 85.154% ± 0.112 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.897% ± 0.068 | 18.222% ± 0.122 | 30.316% ± 0.145 | 69.684% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.473% ± 0.158 | 62.843% ± 0.153 | 73.603% ± 0.139 | 26.397% ± 0.139 |
| Matchup-flex JIT, going second | 51.063% ± 0.158 | 66.446% ± 0.149 | 76.141% ± 0.135 | 23.859% ± 0.135 |
| No discard control, going second | 59.653% ± 0.155 | 74.024% ± 0.139 | 82.430% ± 0.120 | 17.570% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.904% ± 0.080 | 12.837% ± 0.106 | 19.124% ± 0.124 | 80.876% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.292% ± 0.104 | 24.142% ± 0.135 | 33.813% ± 0.150 | 66.187% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.853% ± 0.043 | 6.899% ± 0.080 | 11.867% ± 0.102 | 88.133% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.303% ± 0.077 | 11.189% ± 0.100 | 16.301% ± 0.117 | 83.699% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.616% | 28.290% | 17.727% | 10.944% |
| Matchup-flex JIT, going first | 21.020% | 28.153% | 18.247% | 10.646% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.473% | 15.370% | 10.760% | 6.960% |
| Matchup-flex JIT, going second | 51.063% | 15.383% | 9.695% | 6.473% |
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

Simulator policy digest: `7b33a19d3c17ebc4a29a61efd7d721fcab8187a7957752e1afa915825794f43b`.

Comparison CSV SHA-256: `2b1614484e610f116ac81284ea36e9ae86f707db4a54726a0543860a53652f4f`.
