# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% | 19.552% | +7.559 pp | 39.624% | 48.156% | +8.532 pp | 56.680% | 66.004% | +9.324 pp |
| Strict JIT, going second | 29.643% | 47.612% | +17.969 pp | 53.685% | 63.187% | +9.502 pp | 65.067% | 73.755% | +8.688 pp |
| Matchup-flex JIT, going first | 16.501% | 21.068% | +4.567 pp | 48.271% | 50.031% | +1.760 pp | 64.097% | 68.258% | +4.161 pp |
| Matchup-flex JIT, going second | 37.188% | 51.309% | +14.121 pp | 61.171% | 66.853% | +5.682 pp | 71.450% | 76.520% | +5.070 pp |
| No discard control, going first | 19.977% | 25.196% | +5.219 pp | 56.028% | 60.131% | +4.103 pp | 72.361% | 75.683% | +3.322 pp |
| No discard control, going second | 39.978% | 59.583% | +19.605 pp | 67.118% | 74.005% | +6.887 pp | 78.401% | 82.436% | +4.035 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% ± 0.103 | 39.624% ± 0.155 | 56.680% ± 0.157 | 43.320% ± 0.157 |
| Matchup-flex JIT, going first | 16.501% ± 0.117 | 48.271% ± 0.158 | 64.097% ± 0.152 | 35.903% ± 0.152 |
| No discard control, going first | 19.977% ± 0.126 | 56.028% ± 0.157 | 72.361% ± 0.141 | 27.639% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.300% ± 0.064 | 26.524% ± 0.140 | 40.227% ± 0.155 | 59.773% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.360% ± 0.057 | 7.469% ± 0.083 | 92.531% ± 0.083 |
| Strict JIT, going second | 29.643% ± 0.144 | 53.685% ± 0.158 | 65.067% ± 0.151 | 34.933% ± 0.151 |
| Matchup-flex JIT, going second | 37.188% ± 0.153 | 61.171% ± 0.154 | 71.450% ± 0.143 | 28.550% ± 0.143 |
| No discard control, going second | 39.978% ± 0.155 | 67.118% ± 0.149 | 78.401% ± 0.130 | 21.599% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.221% ± 0.110 | 28.110% ± 0.142 | 35.873% ± 0.152 | 64.127% ± 0.152 |
| Strict JIT, full Item lock, second | 10.564% ± 0.097 | 23.252% ± 0.134 | 30.573% ± 0.146 | 69.427% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.301% ± 0.122 | 35.613% ± 0.151 | 46.155% ± 0.158 | 53.845% ± 0.158 |
| Strict JIT, combined lock, second | 2.510% ± 0.049 | 11.514% ± 0.101 | 15.884% ± 0.116 | 84.116% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.466% ± 0.114 | 21.733% ± 0.130 | 78.267% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.147% ± 0.087 | 19.434% ± 0.125 | 25.319% ± 0.138 | 74.681% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% | 27.631% | 17.056% | 10.585% |
| Matchup-flex JIT, going first | 16.501% | 31.770% | 15.826% | 9.520% |
| No discard control, going first | 19.977% | 36.051% | 16.333% | 9.006% |
| Strict JIT, going second | 29.643% | 24.042% | 11.382% | 7.873% |
| Matchup-flex JIT, going second | 37.188% | 23.983% | 10.279% | 6.946% |
| No discard control, going second | 39.978% | 27.140% | 11.283% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.552% ± 0.125 | 48.156% ± 0.158 | 66.004% ± 0.150 | 33.996% ± 0.150 |
| Matchup-flex JIT, going first | 21.068% ± 0.129 | 50.031% ± 0.158 | 68.258% ± 0.147 | 31.742% ± 0.147 |
| No discard control, going first | 25.196% ± 0.137 | 60.131% ± 0.155 | 75.683% ± 0.136 | 24.317% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.697% ± 0.067 | 8.691% ± 0.089 | 14.846% ± 0.112 | 85.154% ± 0.112 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.866% ± 0.068 | 18.330% ± 0.122 | 30.532% ± 0.146 | 69.468% ± 0.146 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.612% ± 0.158 | 63.187% ± 0.153 | 73.755% ± 0.139 | 26.245% ± 0.139 |
| Matchup-flex JIT, going second | 51.309% ± 0.158 | 66.853% ± 0.149 | 76.520% ± 0.134 | 23.480% ± 0.134 |
| No discard control, going second | 59.583% ± 0.155 | 74.005% ± 0.139 | 82.436% ± 0.120 | 17.564% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.904% ± 0.080 | 12.837% ± 0.106 | 19.124% ± 0.124 | 80.876% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.313% ± 0.104 | 24.248% ± 0.136 | 33.964% ± 0.150 | 66.036% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 6.925% ± 0.080 | 11.905% ± 0.102 | 88.095% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.303% ± 0.077 | 11.200% ± 0.100 | 16.281% ± 0.117 | 83.719% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.552% | 28.604% | 17.848% | 10.516% |
| Matchup-flex JIT, going first | 21.068% | 28.963% | 18.227% | 10.383% |
| No discard control, going first | 25.196% | 34.935% | 15.552% | 7.164% |
| Strict JIT, going second | 47.612% | 15.575% | 10.568% | 7.071% |
| Matchup-flex JIT, going second | 51.309% | 15.544% | 9.667% | 6.508% |
| No discard control, going second | 59.583% | 14.422% | 8.431% | 4.606% |

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
| Secret Box use | 64.363% |
| Exploding Energy use | 78.618% |
| Steven use | 35.767% |
| Star Alchemy use | 48.109% |
| Secret Box attempts | 1.474 per game |
| Cost blocks | 0.049 per game |
| Missing route axis | 0.780 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.046 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.398 |
| Pineco/Forretress line | 0.433 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.155 |
| Known Prize zone | 0.478 |
| Discard zone | 0.240 |
| Stranded hand zone | 0.192 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `d5297a38e286c3a5a3e50b90281a65549cbb38c1d735e6d49b0d10ca9a528738`.

Comparison CSV SHA-256: `45e886cf859d3b2d780b61a9d0b4336a6ee2831fed4f0aa8b47f595349f8e9ec`.
