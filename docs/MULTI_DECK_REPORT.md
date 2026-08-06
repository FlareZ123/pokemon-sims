# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `424242`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.422% | 19.699% | +7.277 pp | 40.152% | 48.225% | +8.073 pp | 57.178% | 66.071% | +8.893 pp |
| Strict JIT, going second | 29.788% | 47.734% | +17.946 pp | 53.670% | 63.519% | +9.849 pp | 65.353% | 74.046% | +8.693 pp |
| Matchup-flex JIT, going first | 16.522% | 20.850% | +4.328 pp | 48.357% | 49.665% | +1.308 pp | 64.386% | 68.075% | +3.689 pp |
| Matchup-flex JIT, going second | 37.519% | 51.204% | +13.685 pp | 61.532% | 66.811% | +5.279 pp | 72.067% | 76.627% | +4.560 pp |
| No discard control, going first | 20.286% | 25.342% | +5.056 pp | 56.177% | 60.342% | +4.165 pp | 72.428% | 75.668% | +3.240 pp |
| No discard control, going second | 40.270% | 59.548% | +19.278 pp | 67.321% | 73.895% | +6.574 pp | 78.659% | 82.361% | +3.702 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.422% ± 0.104 | 40.152% ± 0.155 | 57.178% ± 0.156 | 42.822% ± 0.156 |
| Matchup-flex JIT, going first | 16.522% ± 0.117 | 48.357% ± 0.158 | 64.386% ± 0.151 | 35.614% ± 0.151 |
| No discard control, going first | 20.286% ± 0.127 | 56.177% ± 0.157 | 72.428% ± 0.141 | 27.572% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.408% ± 0.065 | 10.257% ± 0.096 | 18.066% ± 0.122 | 81.934% ± 0.122 |
| Strict JIT, full Item lock, first | 2.904% ± 0.053 | 8.005% ± 0.086 | 15.378% ± 0.114 | 84.622% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.382% ± 0.065 | 26.449% ± 0.139 | 40.209% ± 0.155 | 59.791% ± 0.155 |
| Strict JIT, combined lock, first | 0.344% ± 0.019 | 3.334% ± 0.057 | 7.372% ± 0.083 | 92.628% ± 0.083 |
| Strict JIT, going second | 29.788% ± 0.145 | 53.670% ± 0.158 | 65.353% ± 0.150 | 34.647% ± 0.150 |
| Matchup-flex JIT, going second | 37.519% ± 0.153 | 61.532% ± 0.154 | 72.067% ± 0.142 | 27.933% ± 0.142 |
| No discard control, going second | 40.270% ± 0.155 | 67.321% ± 0.148 | 78.659% ± 0.130 | 21.341% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.053% ± 0.110 | 28.263% ± 0.142 | 36.699% ± 0.152 | 63.301% ± 0.152 |
| Strict JIT, full Item lock, second | 10.716% ± 0.098 | 23.458% ± 0.134 | 31.353% ± 0.147 | 68.647% ± 0.147 |
| Strict JIT, Rule Box Ability lock, second | 18.145% ± 0.122 | 35.410% ± 0.151 | 46.101% ± 0.158 | 53.899% ± 0.158 |
| Strict JIT, combined lock, second | 2.506% ± 0.049 | 11.556% ± 0.101 | 16.006% ± 0.116 | 83.994% ± 0.116 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.315% ± 0.114 | 21.648% ± 0.130 | 78.352% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.407% ± 0.125 | 25.110% ± 0.137 | 74.890% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.422% | 27.730% | 17.026% | 10.750% |
| Matchup-flex JIT, going first | 16.522% | 31.835% | 16.029% | 9.665% |
| No discard control, going first | 20.286% | 35.891% | 16.251% | 9.048% |
| Strict JIT, going second | 29.788% | 23.882% | 11.683% | 8.073% |
| Matchup-flex JIT, going second | 37.519% | 24.013% | 10.535% | 6.905% |
| No discard control, going second | 40.270% | 27.051% | 11.338% | 6.500% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.699% ± 0.126 | 48.225% ± 0.158 | 66.071% ± 0.150 | 33.929% ± 0.150 |
| Matchup-flex JIT, going first | 20.850% ± 0.128 | 49.665% ± 0.158 | 68.075% ± 0.147 | 31.925% ± 0.147 |
| No discard control, going first | 25.342% ± 0.138 | 60.342% ± 0.155 | 75.668% ± 0.136 | 24.332% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.724% ± 0.067 | 8.685% ± 0.089 | 14.865% ± 0.112 | 85.135% ± 0.112 |
| Strict JIT, full Item lock, first | 2.812% ± 0.052 | 6.065% ± 0.075 | 11.501% ± 0.101 | 88.499% ± 0.101 |
| Strict JIT, Rule Box Ability lock, first | 4.760% ± 0.067 | 18.150% ± 0.122 | 30.144% ± 0.145 | 69.856% ± 0.145 |
| Strict JIT, combined lock, first | 0.476% ± 0.022 | 1.469% ± 0.038 | 3.388% ± 0.057 | 96.612% ± 0.057 |
| Strict JIT, going second | 47.734% ± 0.158 | 63.519% ± 0.152 | 74.046% ± 0.139 | 25.954% ± 0.139 |
| Matchup-flex JIT, going second | 51.204% ± 0.158 | 66.811% ± 0.149 | 76.627% ± 0.134 | 23.373% ± 0.134 |
| No discard control, going second | 59.548% ± 0.155 | 73.895% ± 0.139 | 82.361% ± 0.121 | 17.639% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.839% ± 0.080 | 12.870% ± 0.106 | 19.029% ± 0.124 | 80.971% ± 0.124 |
| Strict JIT, full Item lock, second | 4.660% ± 0.067 | 9.633% ± 0.093 | 15.372% ± 0.114 | 84.628% ± 0.114 |
| Strict JIT, Rule Box Ability lock, second | 12.488% ± 0.105 | 24.496% ± 0.136 | 34.348% ± 0.150 | 65.652% ± 0.150 |
| Strict JIT, combined lock, second | 1.210% ± 0.035 | 2.797% ± 0.052 | 4.690% ± 0.067 | 95.310% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.867% ± 0.043 | 6.652% ± 0.079 | 11.620% ± 0.101 | 88.380% ± 0.101 |
| Strict JIT, Supporter lock, second | 6.320% ± 0.077 | 11.162% ± 0.100 | 16.267% ± 0.117 | 83.733% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.699% | 28.526% | 17.846% | 10.547% |
| Matchup-flex JIT, going first | 20.850% | 28.815% | 18.410% | 10.515% |
| No discard control, going first | 25.342% | 35.000% | 15.326% | 7.274% |
| Strict JIT, going second | 47.734% | 15.785% | 10.527% | 6.775% |
| Matchup-flex JIT, going second | 51.204% | 15.607% | 9.816% | 6.304% |
| No discard control, going second | 59.548% | 14.347% | 8.466% | 4.602% |

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
| Secret Box use | 64.339% |
| Exploding Energy use | 78.427% |
| Steven use | 35.904% |
| Star Alchemy use | 48.062% |
| Secret Box attempts | 1.455 per game |
| Cost blocks | 0.047 per game |
| Missing route axis | 0.764 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.303 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.042 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.393 |
| Pineco/Forretress line | 0.418 |
| VSTAR | 0.006 |
| Payload | 0.000 |
| Search Item | 0.002 |
| Fire | 0.022 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.150 |
| Known Prize zone | 0.462 |
| Discard zone | 0.235 |
| Stranded hand zone | 0.189 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `5ff801ce2167b22fbdc2f34fa010c26d9a345e4c310ef9abe69294458a3abfe5`.

Comparison CSV SHA-256: `d163a162441d8ee9a0b12db9acec3408647890c0c39b38fdd3176826343aaaa0`.
