# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.961% | 19.536% | +7.575 pp | 38.632% | 47.324% | +8.692 pp | 55.388% | 65.273% | +9.885 pp |
| Strict JIT, going second | 29.661% | 47.517% | +17.856 pp | 52.676% | 62.850% | +10.174 pp | 63.961% | 73.474% | +9.513 pp |
| Matchup-flex JIT, going first | 16.150% | 20.699% | +4.549 pp | 47.417% | 48.136% | +0.719 pp | 63.424% | 66.520% | +3.096 pp |
| Matchup-flex JIT, going second | 37.463% | 50.611% | +13.148 pp | 61.107% | 65.682% | +4.575 pp | 71.304% | 75.602% | +4.298 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.830% | 59.679% | +19.849 pp | 66.914% | 74.025% | +7.111 pp | 77.999% | 82.431% | +4.432 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.961% ± 0.103 | 38.632% ± 0.154 | 55.388% ± 0.157 | 44.612% ± 0.157 |
| Matchup-flex JIT, going first | 16.150% ± 0.116 | 47.417% ± 0.158 | 63.424% ± 0.152 | 36.576% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.540% ± 0.066 | 10.287% ± 0.096 | 17.767% ± 0.121 | 82.233% ± 0.121 |
| Strict JIT, full Item lock, first | 2.842% ± 0.053 | 7.664% ± 0.084 | 14.878% ± 0.113 | 85.122% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.400% ± 0.065 | 25.783% ± 0.138 | 38.862% ± 0.154 | 61.138% ± 0.154 |
| Strict JIT, combined lock, first | 0.297% ± 0.017 | 3.346% ± 0.057 | 7.345% ± 0.082 | 92.655% ± 0.082 |
| Strict JIT, going second | 29.661% ± 0.144 | 52.676% ± 0.158 | 63.961% ± 0.152 | 36.039% ± 0.152 |
| Matchup-flex JIT, going second | 37.463% ± 0.153 | 61.107% ± 0.154 | 71.304% ± 0.143 | 28.696% ± 0.143 |
| No discard control, going second | 39.830% ± 0.155 | 66.914% ± 0.149 | 77.999% ± 0.131 | 22.001% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 13.945% ± 0.110 | 27.792% ± 0.142 | 35.487% ± 0.151 | 64.513% ± 0.151 |
| Strict JIT, full Item lock, second | 10.545% ± 0.097 | 22.756% ± 0.133 | 29.926% ± 0.145 | 70.074% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.328% ± 0.122 | 34.953% ± 0.151 | 45.183% ± 0.157 | 54.817% ± 0.157 |
| Strict JIT, combined lock, second | 2.369% ± 0.048 | 11.349% ± 0.100 | 15.404% ± 0.114 | 84.596% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.383% ± 0.114 | 21.787% ± 0.131 | 78.213% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.131% ± 0.086 | 19.386% ± 0.125 | 25.491% ± 0.138 | 74.509% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.961% | 26.671% | 16.756% | 10.565% |
| Matchup-flex JIT, going first | 16.150% | 31.267% | 16.007% | 9.629% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.661% | 23.015% | 11.285% | 7.958% |
| Matchup-flex JIT, going second | 37.463% | 23.644% | 10.197% | 6.902% |
| No discard control, going second | 39.830% | 27.084% | 11.085% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.536% ± 0.125 | 47.324% ± 0.158 | 65.273% ± 0.151 | 34.727% ± 0.151 |
| Matchup-flex JIT, going first | 20.699% ± 0.128 | 48.136% ± 0.158 | 66.520% ± 0.149 | 33.480% ± 0.149 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.573% ± 0.066 | 7.783% ± 0.085 | 13.225% ± 0.107 | 86.775% ± 0.107 |
| Strict JIT, full Item lock, first | 2.790% ± 0.052 | 5.702% ± 0.073 | 10.730% ± 0.098 | 89.270% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.911% ± 0.068 | 17.817% ± 0.121 | 29.764% ± 0.145 | 70.236% ± 0.145 |
| Strict JIT, combined lock, first | 0.459% ± 0.021 | 1.344% ± 0.036 | 3.153% ± 0.055 | 96.847% ± 0.055 |
| Strict JIT, going second | 47.517% ± 0.158 | 62.850% ± 0.153 | 73.474% ± 0.140 | 26.526% ± 0.140 |
| Matchup-flex JIT, going second | 50.611% ± 0.158 | 65.682% ± 0.150 | 75.602% ± 0.136 | 24.398% ± 0.136 |
| No discard control, going second | 59.679% ± 0.155 | 74.025% ± 0.139 | 82.431% ± 0.120 | 17.569% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.501% ± 0.078 | 11.738% ± 0.102 | 17.543% ± 0.120 | 82.457% ± 0.120 |
| Strict JIT, full Item lock, second | 4.378% ± 0.065 | 9.151% ± 0.091 | 14.588% ± 0.112 | 85.412% ± 0.112 |
| Strict JIT, Rule Box Ability lock, second | 12.230% ± 0.104 | 23.831% ± 0.135 | 33.375% ± 0.149 | 66.625% ± 0.149 |
| Strict JIT, combined lock, second | 1.130% ± 0.033 | 2.526% ± 0.050 | 4.399% ± 0.065 | 95.601% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.877% ± 0.043 | 6.004% ± 0.075 | 10.482% ± 0.097 | 89.518% ± 0.097 |
| Strict JIT, Supporter lock, second | 6.075% ± 0.076 | 10.099% ± 0.095 | 14.689% ± 0.112 | 85.311% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.536% | 27.788% | 17.949% | 10.861% |
| Matchup-flex JIT, going first | 20.699% | 27.437% | 18.384% | 11.096% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.517% | 15.333% | 10.624% | 7.190% |
| Matchup-flex JIT, going second | 50.611% | 15.071% | 9.920% | 6.887% |
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

Simulator policy digest: `6b184bb144e29cd742a54ecd425cc495d47cd1926be497754592a5188607f2a4`.

Comparison CSV SHA-256: `095f797795eee27cb9bcedd045f2573837f1b02d3669cf30eb9313ab39cda34a`.
