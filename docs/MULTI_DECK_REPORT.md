# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.155% | 19.448% | +7.293 pp | 39.892% | 47.977% | +8.085 pp | 56.780% | 65.991% | +9.211 pp |
| Strict JIT, going second | 30.064% | 47.542% | +17.478 pp | 53.967% | 63.247% | +9.280 pp | 65.245% | 73.836% | +8.591 pp |
| Matchup-flex JIT, going first | 16.497% | 21.024% | +4.527 pp | 48.318% | 49.895% | +1.577 pp | 64.074% | 68.361% | +4.287 pp |
| Matchup-flex JIT, going second | 37.319% | 50.883% | +13.564 pp | 61.378% | 66.801% | +5.423 pp | 71.490% | 76.431% | +4.941 pp |
| No discard control, going first | 19.956% | 25.208% | +5.252 pp | 56.021% | 60.453% | +4.432 pp | 72.357% | 75.830% | +3.473 pp |
| No discard control, going second | 39.964% | 59.437% | +19.473 pp | 67.123% | 73.937% | +6.814 pp | 78.368% | 82.494% | +4.126 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.155% ± 0.103 | 39.892% ± 0.155 | 56.780% ± 0.157 | 43.220% ± 0.157 |
| Matchup-flex JIT, going first | 16.497% ± 0.117 | 48.318% ± 0.158 | 64.074% ± 0.152 | 35.926% ± 0.152 |
| No discard control, going first | 19.956% ± 0.126 | 56.021% ± 0.157 | 72.357% ± 0.141 | 27.643% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.410% ± 0.065 | 26.572% ± 0.140 | 40.162% ± 0.155 | 59.838% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.357% ± 0.057 | 7.468% ± 0.083 | 92.532% ± 0.083 |
| Strict JIT, going second | 30.064% ± 0.145 | 53.967% ± 0.158 | 65.245% ± 0.151 | 34.755% ± 0.151 |
| Matchup-flex JIT, going second | 37.319% ± 0.153 | 61.378% ± 0.154 | 71.490% ± 0.143 | 28.510% ± 0.143 |
| No discard control, going second | 39.964% ± 0.155 | 67.123% ± 0.149 | 78.368% ± 0.130 | 21.632% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.204% ± 0.110 | 28.109% ± 0.142 | 35.870% ± 0.152 | 64.130% ± 0.152 |
| Strict JIT, full Item lock, second | 10.565% ± 0.097 | 23.255% ± 0.134 | 30.566% ± 0.146 | 69.434% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.226% ± 0.122 | 35.522% ± 0.151 | 46.064% ± 0.158 | 53.936% ± 0.158 |
| Strict JIT, combined lock, second | 2.514% ± 0.050 | 11.507% ± 0.101 | 15.879% ± 0.116 | 84.121% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.466% ± 0.114 | 21.733% ± 0.130 | 78.267% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.152% ± 0.087 | 19.442% ± 0.125 | 25.322% ± 0.138 | 74.678% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.155% | 27.737% | 16.888% | 10.484% |
| Matchup-flex JIT, going first | 16.497% | 31.821% | 15.756% | 9.561% |
| No discard control, going first | 19.956% | 36.065% | 16.336% | 9.009% |
| Strict JIT, going second | 30.064% | 23.903% | 11.278% | 7.817% |
| Matchup-flex JIT, going second | 37.319% | 24.059% | 10.112% | 6.911% |
| No discard control, going second | 39.964% | 27.159% | 11.245% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.448% ± 0.125 | 47.977% ± 0.158 | 65.991% ± 0.150 | 34.009% ± 0.150 |
| Matchup-flex JIT, going first | 21.024% ± 0.129 | 49.895% ± 0.158 | 68.361% ± 0.147 | 31.639% ± 0.147 |
| No discard control, going first | 25.208% ± 0.137 | 60.453% ± 0.155 | 75.830% ± 0.135 | 24.170% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.675% ± 0.067 | 8.654% ± 0.089 | 14.881% ± 0.113 | 85.119% ± 0.113 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.953% ± 0.069 | 18.321% ± 0.122 | 30.449% ± 0.146 | 69.551% ± 0.146 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.542% ± 0.158 | 63.247% ± 0.152 | 73.836% ± 0.139 | 26.164% ± 0.139 |
| Matchup-flex JIT, going second | 50.883% ± 0.158 | 66.801% ± 0.149 | 76.431% ± 0.134 | 23.569% ± 0.134 |
| No discard control, going second | 59.437% ± 0.155 | 73.937% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.311% ± 0.104 | 24.327% ± 0.136 | 34.022% ± 0.150 | 65.978% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.779% ± 0.052 | 4.748% ± 0.067 | 95.252% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.925% ± 0.043 | 6.900% ± 0.080 | 11.930% ± 0.103 | 88.070% ± 0.103 |
| Strict JIT, Supporter lock, second | 6.374% ± 0.077 | 11.185% ± 0.100 | 16.338% ± 0.117 | 83.662% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.448% | 28.529% | 18.014% | 10.638% |
| Matchup-flex JIT, going first | 21.024% | 28.871% | 18.466% | 10.394% |
| No discard control, going first | 25.208% | 35.245% | 15.377% | 7.191% |
| Strict JIT, going second | 47.542% | 15.705% | 10.589% | 6.947% |
| Matchup-flex JIT, going second | 50.883% | 15.918% | 9.630% | 6.601% |
| No discard control, going second | 59.437% | 14.500% | 8.557% | 4.634% |

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
| Secret Box use | 64.294% |
| Exploding Energy use | 78.575% |
| Steven use | 35.800% |
| Star Alchemy use | 48.114% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.047 per game |
| Missing route axis | 0.774 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.301 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.401 |
| Pineco/Forretress line | 0.425 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.471 |
| Discard zone | 0.238 |
| Stranded hand zone | 0.189 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `cb09f12c34ad4db03675ca611cc8e0f518dd9c1b52094f4fb5d59f8461e86a76`.

Comparison CSV SHA-256: `967b62d3f12118e22b2bf70e0f92e5bfcd22501ee6c501e5739f08749bac3abd`.
