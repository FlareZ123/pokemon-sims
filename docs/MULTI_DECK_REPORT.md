# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.972% | 19.561% | +7.589 pp | 39.735% | 48.150% | +8.415 pp | 56.653% | 66.123% | +9.470 pp |
| Strict JIT, going second | 29.689% | 47.486% | +17.797 pp | 53.589% | 63.131% | +9.542 pp | 64.865% | 73.717% | +8.852 pp |
| Matchup-flex JIT, going first | 16.480% | 20.951% | +4.471 pp | 48.257% | 49.860% | +1.603 pp | 64.091% | 68.348% | +4.257 pp |
| Matchup-flex JIT, going second | 37.221% | 51.056% | +13.835 pp | 61.223% | 66.815% | +5.592 pp | 71.510% | 76.472% | +4.962 pp |
| No discard control, going first | 19.977% | 25.207% | +5.230 pp | 56.028% | 60.433% | +4.405 pp | 72.361% | 75.806% | +3.445 pp |
| No discard control, going second | 39.978% | 59.428% | +19.450 pp | 67.118% | 73.922% | +6.804 pp | 78.401% | 82.493% | +4.092 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.972% ± 0.103 | 39.735% ± 0.155 | 56.653% ± 0.157 | 43.347% ± 0.157 |
| Matchup-flex JIT, going first | 16.480% ± 0.117 | 48.257% ± 0.158 | 64.091% ± 0.152 | 35.909% ± 0.152 |
| No discard control, going first | 19.977% ± 0.126 | 56.028% ± 0.157 | 72.361% ± 0.141 | 27.639% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.551% ± 0.066 | 10.285% ± 0.096 | 18.044% ± 0.122 | 81.956% ± 0.122 |
| Strict JIT, full Item lock, first | 2.839% ± 0.053 | 7.771% ± 0.085 | 15.260% ± 0.114 | 84.740% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.263% ± 0.064 | 26.404% ± 0.139 | 40.104% ± 0.155 | 59.896% ± 0.155 |
| Strict JIT, combined lock, first | 0.313% ± 0.018 | 3.360% ± 0.057 | 7.469% ± 0.083 | 92.531% ± 0.083 |
| Strict JIT, going second | 29.689% ± 0.144 | 53.589% ± 0.158 | 64.865% ± 0.151 | 35.135% ± 0.151 |
| Matchup-flex JIT, going second | 37.221% ± 0.153 | 61.223% ± 0.154 | 71.510% ± 0.143 | 28.490% ± 0.143 |
| No discard control, going second | 39.978% ± 0.155 | 67.118% ± 0.149 | 78.401% ± 0.130 | 21.599% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.221% ± 0.110 | 28.110% ± 0.142 | 35.873% ± 0.152 | 64.127% ± 0.152 |
| Strict JIT, full Item lock, second | 10.564% ± 0.097 | 23.252% ± 0.134 | 30.573% ± 0.146 | 69.427% ± 0.146 |
| Strict JIT, Rule Box Ability lock, second | 18.388% ± 0.123 | 35.572% ± 0.151 | 45.947% ± 0.158 | 54.053% ± 0.158 |
| Strict JIT, combined lock, second | 2.510% ± 0.049 | 11.514% ± 0.101 | 15.884% ± 0.116 | 84.116% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.446% ± 0.114 | 21.712% ± 0.130 | 78.288% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.165% ± 0.087 | 19.457% ± 0.125 | 25.341% ± 0.138 | 74.659% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.972% | 27.763% | 16.918% | 10.656% |
| Matchup-flex JIT, going first | 16.480% | 31.777% | 15.834% | 9.528% |
| No discard control, going first | 19.977% | 36.051% | 16.333% | 9.006% |
| Strict JIT, going second | 29.689% | 23.900% | 11.276% | 7.903% |
| Matchup-flex JIT, going second | 37.221% | 24.002% | 10.287% | 6.949% |
| No discard control, going second | 39.978% | 27.140% | 11.283% | 6.602% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.561% ± 0.125 | 48.150% ± 0.158 | 66.123% ± 0.150 | 33.877% ± 0.150 |
| Matchup-flex JIT, going first | 20.951% ± 0.129 | 49.860% ± 0.158 | 68.348% ± 0.147 | 31.652% ± 0.147 |
| No discard control, going first | 25.207% ± 0.137 | 60.433% ± 0.155 | 75.806% ± 0.135 | 24.194% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.671% ± 0.067 | 8.645% ± 0.089 | 14.872% ± 0.113 | 85.128% ± 0.113 |
| Strict JIT, full Item lock, first | 2.871% ± 0.053 | 6.109% ± 0.076 | 11.360% ± 0.100 | 88.640% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.925% ± 0.068 | 18.223% ± 0.122 | 30.316% ± 0.145 | 69.684% ± 0.145 |
| Strict JIT, combined lock, first | 0.498% ± 0.022 | 1.447% ± 0.038 | 3.286% ± 0.056 | 96.714% ± 0.056 |
| Strict JIT, going second | 47.486% ± 0.158 | 63.131% ± 0.153 | 73.717% ± 0.139 | 26.283% ± 0.139 |
| Matchup-flex JIT, going second | 51.056% ± 0.158 | 66.815% ± 0.149 | 76.472% ± 0.134 | 23.528% ± 0.134 |
| No discard control, going second | 59.428% ± 0.155 | 73.922% ± 0.139 | 82.493% ± 0.120 | 17.507% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.537% ± 0.066 | 9.469% ± 0.093 | 15.029% ± 0.113 | 84.971% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.314% ± 0.104 | 24.358% ± 0.136 | 34.076% ± 0.150 | 65.924% ± 0.150 |
| Strict JIT, combined lock, second | 1.222% ± 0.035 | 2.780% ± 0.052 | 4.749% ± 0.067 | 95.251% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.923% ± 0.043 | 6.896% ± 0.080 | 11.926% ± 0.102 | 88.074% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.380% ± 0.077 | 11.207% ± 0.100 | 16.356% ± 0.117 | 83.644% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.561% | 28.589% | 17.973% | 10.470% |
| Matchup-flex JIT, going first | 20.951% | 28.909% | 18.488% | 10.395% |
| No discard control, going first | 25.207% | 35.226% | 15.373% | 7.209% |
| Strict JIT, going second | 47.486% | 15.645% | 10.586% | 7.069% |
| Matchup-flex JIT, going second | 51.056% | 15.759% | 9.657% | 6.603% |
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

Simulator policy digest: `57234071ec456525079131fc4d2351a0a290a017ccb35912fe8388a9f2fff14a`.

Comparison CSV SHA-256: `be1ef7b240f8bd6049e9cdc243e49725cee4613b9b00f02a1edc1b191fa87f58`.
