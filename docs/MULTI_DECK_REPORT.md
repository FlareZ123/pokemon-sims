# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.949% | 19.400% | +7.451 pp | 39.553% | 47.976% | +8.423 pp | 56.657% | 66.063% | +9.406 pp |
| Strict JIT, going second | 29.681% | 47.521% | +17.840 pp | 53.793% | 63.165% | +9.372 pp | 65.068% | 73.788% | +8.720 pp |
| Matchup-flex JIT, going first | 16.375% | 20.956% | +4.581 pp | 48.350% | 49.819% | +1.469 pp | 64.141% | 68.302% | +4.161 pp |
| Matchup-flex JIT, going second | 37.177% | 50.997% | +13.820 pp | 61.168% | 66.840% | +5.672 pp | 71.369% | 76.502% | +5.133 pp |
| No discard control, going first | 19.977% | 25.207% | +5.230 pp | 56.028% | 60.433% | +4.405 pp | 72.361% | 75.806% | +3.445 pp |
| No discard control, going second | 39.979% | 59.428% | +19.449 pp | 67.119% | 73.922% | +6.803 pp | 78.401% | 82.493% | +4.092 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.949% ± 0.103 | 39.553% ± 0.155 | 56.657% ± 0.157 | 43.343% ± 0.157 |
| Matchup-flex JIT, going first | 16.375% ± 0.117 | 48.350% ± 0.158 | 64.141% ± 0.152 | 35.859% ± 0.152 |
| No discard control, going first | 19.977% ± 0.126 | 56.028% ± 0.157 | 72.361% ± 0.141 | 27.639% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.341% ± 0.064 | 26.610% ± 0.140 | 40.334% ± 0.155 | 59.666% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.360% ± 0.057 | 7.469% ± 0.083 | 92.531% ± 0.083 |
| Strict JIT, going second | 29.681% ± 0.144 | 53.793% ± 0.158 | 65.068% ± 0.151 | 34.932% ± 0.151 |
| Matchup-flex JIT, going second | 37.177% ± 0.153 | 61.168% ± 0.154 | 71.369% ± 0.143 | 28.631% ± 0.143 |
| No discard control, going second | 39.979% ± 0.155 | 67.119% ± 0.149 | 78.401% ± 0.130 | 21.599% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.204% ± 0.110 | 28.102% ± 0.142 | 35.866% ± 0.152 | 64.134% ± 0.152 |
| Strict JIT, full Item lock, second | 10.566% ± 0.097 | 23.251% ± 0.134 | 30.570% ± 0.146 | 69.430% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.253% ± 0.122 | 35.552% ± 0.151 | 46.117% ± 0.158 | 53.883% ± 0.158 |
| Strict JIT, combined lock, second | 2.513% ± 0.049 | 11.497% ± 0.101 | 15.868% ± 0.116 | 84.132% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.466% ± 0.114 | 21.733% ± 0.130 | 78.267% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.147% ± 0.087 | 19.434% ± 0.125 | 25.319% ± 0.138 | 74.681% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.949% | 27.604% | 17.104% | 10.597% |
| Matchup-flex JIT, going first | 16.375% | 31.975% | 15.791% | 9.513% |
| No discard control, going first | 19.977% | 36.051% | 16.333% | 9.006% |
| Strict JIT, going second | 29.681% | 24.112% | 11.275% | 7.909% |
| Matchup-flex JIT, going second | 37.177% | 23.991% | 10.201% | 6.954% |
| No discard control, going second | 39.979% | 27.140% | 11.282% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.400% ± 0.125 | 47.976% ± 0.158 | 66.063% ± 0.150 | 33.937% ± 0.150 |
| Matchup-flex JIT, going first | 20.956% ± 0.129 | 49.819% ± 0.158 | 68.302% ± 0.147 | 31.698% ± 0.147 |
| No discard control, going first | 25.207% ± 0.137 | 60.433% ± 0.155 | 75.806% ± 0.135 | 24.194% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.671% ± 0.067 | 8.645% ± 0.089 | 14.872% ± 0.113 | 85.128% ± 0.113 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.935% ± 0.068 | 18.292% ± 0.122 | 30.409% ± 0.145 | 69.591% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.521% ± 0.158 | 63.165% ± 0.153 | 73.788% ± 0.139 | 26.212% ± 0.139 |
| Matchup-flex JIT, going second | 50.997% ± 0.158 | 66.840% ± 0.149 | 76.502% ± 0.134 | 23.498% ± 0.134 |
| No discard control, going second | 59.428% ± 0.155 | 73.922% ± 0.139 | 82.493% ± 0.120 | 17.507% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.280% ± 0.104 | 24.308% ± 0.136 | 34.001% ± 0.150 | 65.999% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.748% ± 0.067 | 95.252% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.925% ± 0.043 | 6.900% ± 0.080 | 11.930% ± 0.103 | 88.070% ± 0.103 |
| Strict JIT, Supporter lock, second | 6.374% ± 0.077 | 11.185% ± 0.100 | 16.338% ± 0.117 | 83.662% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.400% | 28.576% | 18.087% | 10.615% |
| Matchup-flex JIT, going first | 20.956% | 28.863% | 18.483% | 10.400% |
| No discard control, going first | 25.207% | 35.226% | 15.373% | 7.209% |
| Strict JIT, going second | 47.521% | 15.644% | 10.623% | 6.978% |
| Matchup-flex JIT, going second | 50.997% | 15.843% | 9.662% | 6.572% |
| No discard control, going second | 59.428% | 14.494% | 8.571% | 4.635% |

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
| Secret Box use | 64.290% |
| Exploding Energy use | 78.581% |
| Steven use | 35.775% |
| Star Alchemy use | 48.121% |
| Secret Box attempts | 1.464 per game |
| Cost blocks | 0.047 per game |
| Missing route axis | 0.773 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.301 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.401 |
| Pineco/Forretress line | 0.424 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.470 |
| Discard zone | 0.237 |
| Stranded hand zone | 0.189 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `636c1e1cf1a533f01eb514f90345544ab62b391192952f50126b0b01ebff2f00`.

Comparison CSV SHA-256: `a650996294d0d950cf1c9501201e8fc24f5896ccdfd3627b13a939d6997005bd`.
