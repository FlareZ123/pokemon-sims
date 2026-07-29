# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.933% | 19.403% | +7.470 pp | 38.500% | 47.047% | +8.547 pp | 55.367% | 65.192% | +9.825 pp |
| Strict JIT, going second | 29.492% | 47.459% | +17.967 pp | 52.693% | 62.635% | +9.942 pp | 63.936% | 73.258% | +9.322 pp |
| Matchup-flex JIT, going first | 16.274% | 20.733% | +4.459 pp | 47.590% | 48.292% | +0.702 pp | 63.396% | 66.701% | +3.305 pp |
| Matchup-flex JIT, going second | 37.223% | 50.879% | +13.656 pp | 60.744% | 65.927% | +5.183 pp | 71.022% | 75.621% | +4.599 pp |
| No discard control, going first | 20.134% | 25.361% | +5.227 pp | 56.000% | 60.065% | +4.065 pp | 72.128% | 75.670% | +3.542 pp |
| No discard control, going second | 39.830% | 59.679% | +19.849 pp | 66.914% | 74.025% | +7.111 pp | 77.999% | 82.431% | +4.432 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.933% ± 0.103 | 38.500% ± 0.154 | 55.367% ± 0.157 | 44.633% ± 0.157 |
| Matchup-flex JIT, going first | 16.274% ± 0.117 | 47.590% ± 0.158 | 63.396% ± 0.152 | 36.604% ± 0.152 |
| No discard control, going first | 20.134% ± 0.127 | 56.000% ± 0.157 | 72.128% ± 0.142 | 27.872% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.596% ± 0.066 | 10.206% ± 0.096 | 17.699% ± 0.121 | 82.301% ± 0.121 |
| Strict JIT, full Item lock, first | 2.823% ± 0.052 | 7.749% ± 0.085 | 15.061% ± 0.113 | 84.939% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.487% ± 0.065 | 25.996% ± 0.139 | 39.084% ± 0.154 | 60.916% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.268% ± 0.082 | 92.732% ± 0.082 |
| Strict JIT, going second | 29.492% ± 0.144 | 52.693% ± 0.158 | 63.936% ± 0.152 | 36.064% ± 0.152 |
| Matchup-flex JIT, going second | 37.223% ± 0.153 | 60.744% ± 0.154 | 71.022% ± 0.143 | 28.978% ± 0.143 |
| No discard control, going second | 39.830% ± 0.155 | 66.914% ± 0.149 | 77.999% ± 0.131 | 22.001% ± 0.131 |
| Strict JIT, turn-two Item lock, second | 14.085% ± 0.110 | 27.964% ± 0.142 | 35.542% ± 0.151 | 64.458% ± 0.151 |
| Strict JIT, full Item lock, second | 10.556% ± 0.097 | 22.936% ± 0.133 | 30.105% ± 0.145 | 69.895% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.123% ± 0.122 | 34.679% ± 0.151 | 44.802% ± 0.157 | 55.198% ± 0.157 |
| Strict JIT, combined lock, second | 2.373% ± 0.048 | 11.415% ± 0.101 | 15.519% ± 0.115 | 84.481% ± 0.115 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 15.385% ± 0.114 | 21.677% ± 0.130 | 78.323% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.181% ± 0.087 | 19.626% ± 0.126 | 25.447% ± 0.138 | 74.553% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.933% | 26.567% | 16.867% | 10.711% |
| Matchup-flex JIT, going first | 16.274% | 31.316% | 15.806% | 9.620% |
| No discard control, going first | 20.134% | 35.866% | 16.128% | 9.006% |
| Strict JIT, going second | 29.492% | 23.201% | 11.243% | 7.912% |
| Matchup-flex JIT, going second | 37.223% | 23.521% | 10.278% | 6.994% |
| No discard control, going second | 39.830% | 27.084% | 11.085% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.403% ± 0.125 | 47.047% ± 0.158 | 65.192% ± 0.151 | 34.808% ± 0.151 |
| Matchup-flex JIT, going first | 20.733% ± 0.128 | 48.292% ± 0.158 | 66.701% ± 0.149 | 33.299% ± 0.149 |
| No discard control, going first | 25.361% ± 0.138 | 60.065% ± 0.155 | 75.670% ± 0.136 | 24.330% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.504% ± 0.066 | 7.700% ± 0.084 | 13.274% ± 0.107 | 86.726% ± 0.107 |
| Strict JIT, full Item lock, first | 2.776% ± 0.052 | 5.693% ± 0.073 | 10.734% ± 0.098 | 89.266% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.871% ± 0.068 | 17.775% ± 0.121 | 29.798% ± 0.145 | 70.202% ± 0.145 |
| Strict JIT, combined lock, first | 0.478% ± 0.022 | 1.344% ± 0.036 | 3.102% ± 0.055 | 96.898% ± 0.055 |
| Strict JIT, going second | 47.459% ± 0.158 | 62.635% ± 0.153 | 73.258% ± 0.140 | 26.742% ± 0.140 |
| Matchup-flex JIT, going second | 50.879% ± 0.158 | 65.927% ± 0.150 | 75.621% ± 0.136 | 24.379% ± 0.136 |
| No discard control, going second | 59.679% ± 0.155 | 74.025% ± 0.139 | 82.431% ± 0.120 | 17.569% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.610% ± 0.079 | 11.855% ± 0.102 | 17.576% ± 0.120 | 82.424% ± 0.120 |
| Strict JIT, full Item lock, second | 4.394% ± 0.065 | 9.124% ± 0.091 | 14.468% ± 0.111 | 85.532% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 12.136% ± 0.103 | 23.743% ± 0.135 | 33.386% ± 0.149 | 66.614% ± 0.149 |
| Strict JIT, combined lock, second | 1.163% ± 0.034 | 2.605% ± 0.050 | 4.447% ± 0.065 | 95.553% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 6.011% ± 0.075 | 10.587% ± 0.097 | 89.413% ± 0.097 |
| Strict JIT, Supporter lock, second | 6.092% ± 0.076 | 10.114% ± 0.095 | 14.827% ± 0.112 | 85.173% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.403% | 27.644% | 18.145% | 10.981% |
| Matchup-flex JIT, going first | 20.733% | 27.559% | 18.409% | 11.060% |
| No discard control, going first | 25.361% | 34.704% | 15.605% | 7.181% |
| Strict JIT, going second | 47.459% | 15.176% | 10.623% | 7.271% |
| Matchup-flex JIT, going second | 50.879% | 15.048% | 9.694% | 6.770% |
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

Simulator policy digest: `100568c89d30b221a2b20670685ace1616eeead1f63618a62bfc2e8fc7c4c11b`.

Comparison CSV SHA-256: `36c4e3020429437eca7005fd5edec332d5a8b49c0c0749d5ac36996f57c9898b`.
