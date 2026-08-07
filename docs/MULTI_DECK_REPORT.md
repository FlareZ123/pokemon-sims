# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% | 19.510% | +7.377 pp | 40.819% | 48.512% | +7.693 pp | 57.565% | 66.212% | +8.647 pp |
| Strict JIT, going second | 29.734% | 47.303% | +17.569 pp | 53.946% | 63.333% | +9.387 pp | 65.348% | 73.988% | +8.640 pp |
| Matchup-flex JIT, going first | 16.522% | 21.132% | +4.610 pp | 48.970% | 50.246% | +1.276 pp | 64.706% | 68.421% | +3.715 pp |
| Matchup-flex JIT, going second | 37.574% | 51.148% | +13.574 pp | 62.065% | 66.996% | +4.931 pp | 72.509% | 76.587% | +4.078 pp |
| No discard control, going first | 19.958% | 25.216% | +5.258 pp | 56.020% | 60.418% | +4.398 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.123% | 73.939% | +6.816 pp | 78.368% | 82.494% | +4.126 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% ± 0.103 | 40.819% ± 0.155 | 57.565% ± 0.156 | 42.435% ± 0.156 |
| Matchup-flex JIT, going first | 16.522% ± 0.117 | 48.970% ± 0.158 | 64.706% ± 0.151 | 35.294% ± 0.151 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.393% ± 0.097 | 18.262% ± 0.122 | 81.738% ± 0.122 |
| Strict JIT, full Item lock, first | 2.850% ± 0.053 | 7.890% ± 0.085 | 15.425% ± 0.114 | 84.575% ± 0.114 |
| Strict JIT, Rule Box Ability lock, first | 4.392% ± 0.065 | 26.663% ± 0.140 | 40.243% ± 0.155 | 59.757% ± 0.155 |
| Strict JIT, combined lock, first | 0.308% ± 0.018 | 3.371% ± 0.057 | 7.514% ± 0.083 | 92.486% ± 0.083 |
| Strict JIT, going second | 29.734% ± 0.145 | 53.946% ± 0.158 | 65.348% ± 0.150 | 34.652% ± 0.150 |
| Matchup-flex JIT, going second | 37.574% ± 0.153 | 62.065% ± 0.153 | 72.509% ± 0.141 | 27.491% ± 0.141 |
| No discard control, going second | 39.964% ± 0.155 | 67.123% ± 0.149 | 78.368% ± 0.130 | 21.632% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.177% ± 0.110 | 28.393% ± 0.143 | 36.916% ± 0.153 | 63.084% ± 0.153 |
| Strict JIT, full Item lock, second | 10.515% ± 0.097 | 23.371% ± 0.134 | 31.330% ± 0.147 | 68.670% ± 0.147 |
| Strict JIT, Rule Box Ability lock, second | 18.214% ± 0.122 | 35.658% ± 0.151 | 46.258% ± 0.158 | 53.742% ± 0.158 |
| Strict JIT, combined lock, second | 2.510% ± 0.049 | 11.587% ± 0.101 | 16.268% ± 0.117 | 83.732% ± 0.117 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.382% ± 0.114 | 21.683% ± 0.130 | 78.317% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.123% ± 0.086 | 19.412% ± 0.125 | 25.305% ± 0.137 | 74.695% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% | 28.686% | 16.746% | 10.559% |
| Matchup-flex JIT, going first | 16.522% | 32.448% | 15.736% | 9.644% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.734% | 24.212% | 11.402% | 8.014% |
| Matchup-flex JIT, going second | 37.574% | 24.491% | 10.444% | 6.714% |
| No discard control, going second | 39.964% | 27.159% | 11.245% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.510% ± 0.125 | 48.512% ± 0.158 | 66.212% ± 0.150 | 33.788% ± 0.150 |
| Matchup-flex JIT, going first | 21.132% ± 0.129 | 50.246% ± 0.158 | 68.421% ± 0.147 | 31.579% ± 0.147 |
| No discard control, going first | 25.216% ± 0.137 | 60.418% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.673% ± 0.067 | 8.673% ± 0.089 | 14.888% ± 0.113 | 85.112% ± 0.113 |
| Strict JIT, full Item lock, first | 2.870% ± 0.053 | 6.105% ± 0.076 | 11.355% ± 0.100 | 88.645% ± 0.100 |
| Strict JIT, Rule Box Ability lock, first | 4.958% ± 0.069 | 18.505% ± 0.123 | 30.631% ± 0.146 | 69.369% ± 0.146 |
| Strict JIT, combined lock, first | 0.502% ± 0.022 | 1.448% ± 0.038 | 3.288% ± 0.056 | 96.712% ± 0.056 |
| Strict JIT, going second | 47.303% ± 0.158 | 63.333% ± 0.152 | 73.988% ± 0.139 | 26.012% ± 0.139 |
| Matchup-flex JIT, going second | 51.148% ± 0.158 | 66.996% ± 0.149 | 76.587% ± 0.134 | 23.413% ± 0.134 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, full Item lock, second | 4.543% ± 0.066 | 9.476% ± 0.093 | 15.035% ± 0.113 | 84.965% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 12.442% ± 0.104 | 24.635% ± 0.136 | 34.306% ± 0.150 | 65.694% ± 0.150 |
| Strict JIT, combined lock, second | 1.220% ± 0.035 | 2.774% ± 0.052 | 4.741% ± 0.067 | 95.259% ± 0.067 |
| Strict JIT, Supporter lock, first | 1.924% ± 0.043 | 6.895% ± 0.080 | 11.923% ± 0.102 | 88.077% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.376% ± 0.077 | 11.196% ± 0.100 | 16.342% ± 0.117 | 83.658% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.510% | 29.002% | 17.700% | 10.577% |
| Matchup-flex JIT, going first | 21.132% | 29.114% | 18.175% | 10.491% |
| No discard control, going first | 25.216% | 35.202% | 15.387% | 7.192% |
| Strict JIT, going second | 47.303% | 16.030% | 10.655% | 6.884% |
| Matchup-flex JIT, going second | 51.148% | 15.848% | 9.591% | 6.437% |
| No discard control, going second | 59.434% | 14.505% | 8.555% | 4.635% |

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
| Secret Box use | 64.301% |
| Exploding Energy use | 78.576% |
| Steven use | 35.811% |
| Star Alchemy use | 48.117% |
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

Simulator policy digest: `1d89aa53225a8860ceb8ce1613b6d36295531f987f7d989278197b4c5ccfe325`.

Comparison CSV SHA-256: `3cfb4d882338d957550bc1ec679394febb54a6cce2f2c401dcb8b7265ad69db4`.
