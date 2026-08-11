# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% | 19.820% | +7.687 pp | 41.504% | 49.067% | +7.563 pp | 58.959% | 66.591% | +7.632 pp |
| Strict JIT, going second | 29.696% | 48.373% | +18.677 pp | 54.785% | 64.310% | +9.525 pp | 66.891% | 74.680% | +7.789 pp |
| Matchup-flex JIT, going first | 16.988% | 21.368% | +4.380 pp | 50.302% | 50.603% | +0.301 pp | 67.091% | 68.430% | +1.339 pp |
| Matchup-flex JIT, going second | 37.383% | 51.316% | +13.933 pp | 63.108% | 67.175% | +4.067 pp | 74.461% | 76.773% | +2.312 pp |
| No discard control, going first | 19.747% | 25.207% | +5.460 pp | 56.986% | 60.410% | +3.424 pp | 73.750% | 75.805% | +2.055 pp |
| No discard control, going second | 39.935% | 59.434% | +19.499 pp | 68.330% | 73.939% | +5.609 pp | 79.952% | 82.494% | +2.542 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% ± 0.103 | 41.504% ± 0.156 | 58.959% ± 0.156 | 41.041% ± 0.156 |
| Matchup-flex JIT, going first | 16.988% ± 0.119 | 50.302% ± 0.158 | 67.091% ± 0.149 | 32.909% ± 0.149 |
| No discard control, going first | 19.747% ± 0.126 | 56.986% ± 0.157 | 73.750% ± 0.139 | 26.250% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.575% ± 0.066 | 10.488% ± 0.097 | 18.444% ± 0.123 | 81.556% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.594% ± 0.066 | 27.234% ± 0.141 | 41.615% ± 0.156 | 58.385% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.781% ± 0.028 | 4.648% ± 0.067 | 9.655% ± 0.093 | 90.345% ± 0.093 |
| Strict JIT, going second | 29.696% ± 0.144 | 54.785% ± 0.157 | 66.891% ± 0.149 | 33.109% ± 0.149 |
| Matchup-flex JIT, going second | 37.383% ± 0.153 | 63.108% ± 0.153 | 74.461% ± 0.138 | 25.539% ± 0.138 |
| No discard control, going second | 39.935% ± 0.155 | 68.330% ± 0.147 | 79.952% ± 0.127 | 20.048% ± 0.127 |
| Strict JIT, turn-two Item lock, second | 14.141% ± 0.110 | 28.427% ± 0.143 | 36.845% ± 0.153 | 63.155% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.528% ± 0.123 | 36.608% ± 0.152 | 48.266% ± 0.158 | 51.734% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.785% ± 0.060 | 14.459% ± 0.111 | 19.967% ± 0.126 | 80.033% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 14.769% ± 0.112 | 21.486% ± 0.130 | 78.514% ± 0.130 |
| Strict JIT, Supporter lock, second | 7.971% ± 0.086 | 19.169% ± 0.124 | 25.417% ± 0.138 | 74.583% ± 0.138 |
| Garbodor + Boost Shake Ability lock, first | 5.795% ± 0.074 | 27.112% ± 0.141 | 40.920% ± 0.155 | 59.080% ± 0.155 |
| Garbodor + Boost Shake Ability lock, second | 17.441% ± 0.120 | 34.547% ± 0.150 | 46.341% ± 0.158 | 53.659% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.133% | 29.371% | 17.455% | 10.920% |
| Matchup-flex JIT, going first | 16.988% | 33.314% | 16.789% | 9.836% |
| No discard control, going first | 19.747% | 37.239% | 16.764% | 8.982% |
| Strict JIT, going second | 29.696% | 25.089% | 12.106% | 8.049% |
| Matchup-flex JIT, going second | 37.383% | 25.725% | 11.353% | 7.098% |
| No discard control, going second | 39.935% | 28.395% | 11.622% | 6.493% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.820% ± 0.126 | 49.067% ± 0.158 | 66.591% ± 0.149 | 33.409% ± 0.149 |
| Matchup-flex JIT, going first | 21.368% ± 0.130 | 50.603% ± 0.158 | 68.430% ± 0.147 | 31.570% ± 0.147 |
| No discard control, going first | 25.207% ± 0.137 | 60.410% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.656% ± 0.067 | 8.665% ± 0.089 | 14.898% ± 0.113 | 85.102% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.759% ± 0.067 | 18.355% ± 0.122 | 30.421% ± 0.145 | 69.579% ± 0.145 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.903% ± 0.030 | 2.287% ± 0.047 | 4.776% ± 0.067 | 95.224% ± 0.067 |
| Strict JIT, going second | 48.373% ± 0.158 | 64.310% ± 0.151 | 74.680% ± 0.138 | 25.320% ± 0.138 |
| Matchup-flex JIT, going second | 51.316% ± 0.158 | 67.175% ± 0.148 | 76.773% ± 0.134 | 23.227% ± 0.134 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.866% ± 0.080 | 12.754% ± 0.105 | 19.120% ± 0.124 | 80.880% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.506% ± 0.105 | 24.678% ± 0.136 | 34.496% ± 0.150 | 65.504% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.903% ± 0.043 | 4.399% ± 0.065 | 6.943% ± 0.080 | 93.057% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.361% ± 0.083 | 12.495% ± 0.105 | 87.505% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.551% ± 0.078 | 11.663% ± 0.102 | 16.865% ± 0.118 | 83.135% ± 0.118 |
| Garbodor + Boost Shake Ability lock, first | 6.854% ± 0.080 | 21.264% ± 0.129 | 33.560% ± 0.149 | 66.440% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 12.791% ± 0.106 | 25.337% ± 0.138 | 35.240% ± 0.151 | 64.760% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.820% | 29.247% | 17.524% | 10.448% |
| Matchup-flex JIT, going first | 21.368% | 29.235% | 17.827% | 10.321% |
| No discard control, going first | 25.207% | 35.203% | 15.395% | 7.194% |
| Strict JIT, going second | 48.373% | 15.937% | 10.370% | 6.720% |
| Matchup-flex JIT, going second | 51.316% | 15.859% | 9.598% | 6.441% |
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

Simulator policy digest: `c6020aaa85cb16826ace5cf7daf3127007fc2f87797b2691fa9f8c6c5ad85f28`.

Comparison CSV SHA-256: `ad07e0838dfec35334a6f01b738db55832c672a054769038da8990eb56eb8a12`.
