# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.948% | 19.394% | +7.446 pp | 38.519% | 47.036% | +8.517 pp | 55.388% | 65.166% | +9.778 pp |
| Strict JIT, going second | 29.477% | 47.545% | +18.068 pp | 52.685% | 62.705% | +10.020 pp | 63.930% | 73.294% | +9.364 pp |
| Matchup-flex JIT, going first | 16.265% | 20.719% | +4.454 pp | 47.590% | 48.298% | +0.708 pp | 63.406% | 66.709% | +3.303 pp |
| Matchup-flex JIT, going second | 37.242% | 50.888% | +13.646 pp | 60.739% | 65.938% | +5.199 pp | 71.008% | 75.632% | +4.624 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.830% | 59.679% | +19.849 pp | 66.914% | 74.025% | +7.111 pp | 77.999% | 82.431% | +4.432 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.948% ± 0.103 | 38.519% ± 0.154 | 55.388% ± 0.157 | 44.612% ± 0.157 |
| Matchup-flex JIT, going first | 16.265% ± 0.117 | 47.590% ± 0.158 | 63.406% ± 0.152 | 36.594% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.596% ± 0.066 | 10.207% ± 0.096 | 17.726% ± 0.121 | 82.274% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.475% ± 0.065 | 25.996% ± 0.139 | 39.082% ± 0.154 | 60.918% ± 0.154 |
| Strict JIT, combined lock, first | 0.287% ± 0.017 | 3.278% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 29.477% ± 0.144 | 52.685% ± 0.158 | 63.930% ± 0.152 | 36.070% ± 0.152 |
| Matchup-flex JIT, going second | 37.242% ± 0.153 | 60.739% ± 0.154 | 71.008% ± 0.143 | 28.992% ± 0.143 |
| No discard control, going second | 39.830% ± 0.155 | 66.914% ± 0.149 | 77.999% ± 0.131 | 22.001% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.039% ± 0.110 | 27.907% ± 0.142 | 35.551% ± 0.151 | 64.449% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.128% ± 0.122 | 34.666% ± 0.150 | 44.774% ± 0.157 | 55.226% ± 0.157 |
| Strict JIT, combined lock, second | 2.370% ± 0.048 | 11.414% ± 0.101 | 15.520% ± 0.115 | 84.480% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 15.364% ± 0.114 | 21.654% ± 0.130 | 78.346% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.182% ± 0.087 | 19.620% ± 0.126 | 25.442% ± 0.138 | 74.558% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.948% | 26.571% | 16.869% | 10.706% |
| Matchup-flex JIT, going first | 16.265% | 31.325% | 15.816% | 9.625% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.477% | 23.208% | 11.245% | 7.915% |
| Matchup-flex JIT, going second | 37.242% | 23.497% | 10.269% | 7.009% |
| No discard control, going second | 39.830% | 27.084% | 11.085% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.394% ± 0.125 | 47.036% ± 0.158 | 65.166% ± 0.151 | 34.834% ± 0.151 |
| Matchup-flex JIT, going first | 20.719% ± 0.128 | 48.298% ± 0.158 | 66.709% ± 0.149 | 33.291% ± 0.149 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.870% ± 0.068 | 17.748% ± 0.121 | 29.766% ± 0.145 | 70.234% ± 0.145 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 47.545% ± 0.158 | 62.705% ± 0.153 | 73.294% ± 0.140 | 26.706% ± 0.140 |
| Matchup-flex JIT, going second | 50.888% ± 0.158 | 65.938% ± 0.150 | 75.632% ± 0.136 | 24.368% ± 0.136 |
| No discard control, going second | 59.679% ± 0.155 | 74.025% ± 0.139 | 82.431% ± 0.120 | 17.569% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.605% ± 0.079 | 11.845% ± 0.102 | 17.561% ± 0.120 | 82.439% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 12.142% ± 0.103 | 23.759% ± 0.135 | 33.395% ± 0.149 | 66.605% ± 0.149 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 6.038% ± 0.075 | 10.598% ± 0.097 | 89.402% ± 0.097 |
| Strict JIT, Supporter lock, second | 6.095% ± 0.076 | 10.123% ± 0.095 | 14.832% ± 0.112 | 85.168% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.394% | 27.642% | 18.130% | 10.977% |
| Matchup-flex JIT, going first | 20.719% | 27.579% | 18.411% | 11.052% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.545% | 15.160% | 10.589% | 7.257% |
| Matchup-flex JIT, going second | 50.888% | 15.050% | 9.694% | 6.772% |
| No discard control, going second | 59.679% | 14.346% | 8.406% | 4.604% |

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
| Secret Box use | 64.366% |
| Exploding Energy use | 78.626% |
| Steven use | 35.704% |
| Star Alchemy use | 48.086% |
| Secret Box attempts | 1.470 per game |
| Cost blocks | 0.048 per game |
| Missing route axis | 0.777 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.046 per game |

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

Simulator policy digest: `c4e0b78f56ed72b84df5b0204f1b23a330bff30c5c3e55a19af9b4c277d5e37b`.

Comparison CSV SHA-256: `2c7dda33bf4a88448040122c1c61aa6a4db53b3e5bfe3ede74610030da67b81e`.
