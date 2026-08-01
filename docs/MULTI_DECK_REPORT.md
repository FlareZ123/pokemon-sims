# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.087% | 19.433% | +7.346 pp | 39.762% | 47.949% | +8.187 pp | 56.748% | 65.954% | +9.206 pp |
| Strict JIT, going second | 30.036% | 47.503% | +17.467 pp | 53.945% | 63.219% | +9.274 pp | 65.274% | 73.773% | +8.499 pp |
| Matchup-flex JIT, going first | 16.497% | 21.027% | +4.530 pp | 48.286% | 49.901% | +1.615 pp | 64.064% | 68.360% | +4.296 pp |
| Matchup-flex JIT, going second | 37.318% | 50.874% | +13.556 pp | 61.368% | 66.778% | +5.410 pp | 71.505% | 76.431% | +4.926 pp |
| No discard control, going first | 19.977% | 25.207% | +5.230 pp | 56.028% | 60.433% | +4.405 pp | 72.361% | 75.806% | +3.445 pp |
| No discard control, going second | 39.979% | 59.428% | +19.449 pp | 67.119% | 73.922% | +6.803 pp | 78.401% | 82.493% | +4.092 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.087% ± 0.103 | 39.762% ± 0.155 | 56.748% ± 0.157 | 43.252% ± 0.157 |
| Matchup-flex JIT, going first | 16.497% ± 0.117 | 48.286% ± 0.158 | 64.064% ± 0.152 | 35.936% ± 0.152 |
| No discard control, going first | 19.977% ± 0.126 | 56.028% ± 0.157 | 72.361% ± 0.141 | 27.639% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.407% ± 0.065 | 26.618% ± 0.140 | 40.185% ± 0.155 | 59.815% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.360% ± 0.057 | 7.469% ± 0.083 | 92.531% ± 0.083 |
| Strict JIT, going second | 30.036% ± 0.145 | 53.945% ± 0.158 | 65.274% ± 0.151 | 34.726% ± 0.151 |
| Matchup-flex JIT, going second | 37.318% ± 0.153 | 61.368% ± 0.154 | 71.505% ± 0.143 | 28.495% ± 0.143 |
| No discard control, going second | 39.979% ± 0.155 | 67.119% ± 0.149 | 78.401% ± 0.130 | 21.599% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.204% ± 0.110 | 28.102% ± 0.142 | 35.866% ± 0.152 | 64.134% ± 0.152 |
| Strict JIT, full Item lock, second | 10.566% ± 0.097 | 23.251% ± 0.134 | 30.570% ± 0.146 | 69.430% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.240% ± 0.122 | 35.531% ± 0.151 | 46.067% ± 0.158 | 53.933% ± 0.158 |
| Strict JIT, combined lock, second | 2.513% ± 0.049 | 11.497% ± 0.101 | 15.868% ± 0.116 | 84.132% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.466% ± 0.114 | 21.733% ± 0.130 | 78.267% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.147% ± 0.087 | 19.434% ± 0.125 | 25.319% ± 0.138 | 74.681% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.087% | 27.675% | 16.986% | 10.436% |
| Matchup-flex JIT, going first | 16.497% | 31.789% | 15.778% | 9.552% |
| No discard control, going first | 19.977% | 36.051% | 16.333% | 9.006% |
| Strict JIT, going second | 30.036% | 23.909% | 11.329% | 7.791% |
| Matchup-flex JIT, going second | 37.318% | 24.050% | 10.137% | 6.910% |
| No discard control, going second | 39.979% | 27.140% | 11.282% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.433% ± 0.125 | 47.949% ± 0.158 | 65.954% ± 0.150 | 34.046% ± 0.150 |
| Matchup-flex JIT, going first | 21.027% ± 0.129 | 49.901% ± 0.158 | 68.360% ± 0.147 | 31.640% ± 0.147 |
| No discard control, going first | 25.207% ± 0.137 | 60.433% ± 0.155 | 75.806% ± 0.135 | 24.194% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.671% ± 0.067 | 8.645% ± 0.089 | 14.872% ± 0.113 | 85.128% ± 0.113 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.947% ± 0.069 | 18.297% ± 0.122 | 30.420% ± 0.145 | 69.580% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.503% ± 0.158 | 63.219% ± 0.152 | 73.773% ± 0.139 | 26.227% ± 0.139 |
| Matchup-flex JIT, going second | 50.874% ± 0.158 | 66.778% ± 0.149 | 76.431% ± 0.134 | 23.569% ± 0.134 |
| No discard control, going second | 59.428% ± 0.155 | 73.922% ± 0.139 | 82.493% ± 0.120 | 17.507% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.311% ± 0.104 | 24.327% ± 0.136 | 34.022% ± 0.150 | 65.978% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.748% ± 0.067 | 95.252% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.925% ± 0.043 | 6.900% ± 0.080 | 11.930% ± 0.103 | 88.070% ± 0.103 |
| Strict JIT, Supporter lock, second | 6.374% ± 0.077 | 11.185% ± 0.100 | 16.338% ± 0.117 | 83.662% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.433% | 28.516% | 18.005% | 10.632% |
| Matchup-flex JIT, going first | 21.027% | 28.874% | 18.459% | 10.390% |
| No discard control, going first | 25.207% | 35.226% | 15.373% | 7.209% |
| Strict JIT, going second | 47.503% | 15.716% | 10.554% | 6.918% |
| Matchup-flex JIT, going second | 50.874% | 15.904% | 9.653% | 6.617% |
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

Simulator policy digest: `a542ce5e2c66c76c108dc0331345e352c1caf478f363806d35bbbb4dc5b3297e`.

Comparison CSV SHA-256: `bf61059115ced446d77e85d5db9f4a53d36c2ad987fc45873e398b6577d36ba9`.
