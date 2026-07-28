# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.039% | 19.380% | +7.341 pp | 38.603% | 47.051% | +8.448 pp | 55.424% | 65.171% | +9.747 pp |
| Strict JIT, going second | 29.383% | 47.580% | +18.197 pp | 52.581% | 62.716% | +10.135 pp | 63.832% | 73.294% | +9.462 pp |
| Matchup-flex JIT, going first | 16.299% | 20.749% | +4.450 pp | 47.621% | 48.324% | +0.703 pp | 63.434% | 66.730% | +3.296 pp |
| Matchup-flex JIT, going second | 37.320% | 50.861% | +13.541 pp | 60.800% | 65.890% | +5.090 pp | 71.034% | 75.608% | +4.574 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.841% | 59.679% | +19.838 pp | 66.845% | 74.025% | +7.180 pp | 77.988% | 82.431% | +4.443 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.039% ± 0.103 | 38.603% ± 0.154 | 55.424% ± 0.157 | 44.576% ± 0.157 |
| Matchup-flex JIT, going first | 16.299% ± 0.117 | 47.621% ± 0.158 | 63.434% ± 0.152 | 36.566% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.441% ± 0.065 | 25.965% ± 0.139 | 39.028% ± 0.154 | 60.972% ± 0.154 |
| Strict JIT, combined lock, first | 0.287% ± 0.017 | 3.278% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 29.383% ± 0.144 | 52.581% ± 0.158 | 63.832% ± 0.152 | 36.168% ± 0.152 |
| Matchup-flex JIT, going second | 37.320% ± 0.153 | 60.800% ± 0.154 | 71.034% ± 0.143 | 28.966% ± 0.143 |
| No discard control, going second | 39.841% ± 0.155 | 66.845% ± 0.149 | 77.988% ± 0.131 | 22.012% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.086% ± 0.110 | 27.941% ± 0.142 | 35.593% ± 0.151 | 64.407% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.093% ± 0.122 | 34.622% ± 0.150 | 44.730% ± 0.157 | 55.270% ± 0.157 |
| Strict JIT, combined lock, second | 2.370% ± 0.048 | 11.414% ± 0.101 | 15.520% ± 0.115 | 84.480% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.289% ± 0.114 | 21.545% ± 0.130 | 78.455% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.122% ± 0.086 | 19.457% ± 0.125 | 25.347% ± 0.138 | 74.653% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.039% | 26.564% | 16.821% | 10.644% |
| Matchup-flex JIT, going first | 16.299% | 31.322% | 15.813% | 9.580% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.383% | 23.198% | 11.251% | 7.972% |
| Matchup-flex JIT, going second | 37.320% | 23.480% | 10.234% | 6.999% |
| No discard control, going second | 39.841% | 27.004% | 11.143% | 6.675% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.380% ± 0.125 | 47.051% ± 0.158 | 65.171% ± 0.151 | 34.829% ± 0.151 |
| Matchup-flex JIT, going first | 20.749% ± 0.128 | 48.324% ± 0.158 | 66.730% ± 0.149 | 33.270% ± 0.149 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.870% ± 0.068 | 17.748% ± 0.121 | 29.766% ± 0.145 | 70.234% ± 0.145 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 47.580% ± 0.158 | 62.716% ± 0.153 | 73.294% ± 0.140 | 26.706% ± 0.140 |
| Matchup-flex JIT, going second | 50.861% ± 0.158 | 65.890% ± 0.150 | 75.608% ± 0.136 | 24.392% ± 0.136 |
| No discard control, going second | 59.679% ± 0.155 | 74.025% ± 0.139 | 82.431% ± 0.120 | 17.569% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.605% ± 0.079 | 11.845% ± 0.102 | 17.561% ± 0.120 | 82.439% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 12.160% ± 0.103 | 23.772% ± 0.135 | 33.403% ± 0.149 | 66.597% ± 0.149 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 6.038% ± 0.075 | 10.598% ± 0.097 | 89.402% ± 0.097 |
| Strict JIT, Supporter lock, second | 6.095% ± 0.076 | 10.123% ± 0.095 | 14.832% ± 0.112 | 85.168% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.380% | 27.671% | 18.120% | 10.983% |
| Matchup-flex JIT, going first | 20.749% | 27.575% | 18.406% | 11.023% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.580% | 15.136% | 10.578% | 7.257% |
| Matchup-flex JIT, going second | 50.861% | 15.029% | 9.718% | 6.752% |
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

Simulator policy digest: `560164e35dab997c61b8344e5c365421fbf1e67aeffa6d0d18b845e3290a1e3a`.

Comparison CSV SHA-256: `bc22660fdf8418a240030fa2816d6e815a470dca98337a2d0eb392797fbfac3b`.
