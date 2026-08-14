# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.204% | 19.618% | +7.414 pp | 41.795% | 49.028% | +7.233 pp | 59.098% | 66.763% | +7.665 pp |
| Strict JIT, going second | 29.752% | 48.445% | +18.693 pp | 55.420% | 64.490% | +9.070 pp | 67.361% | 74.933% | +7.572 pp |
| Matchup-flex JIT, going first | 16.917% | 21.175% | +4.258 pp | 50.069% | 50.717% | +0.648 pp | 67.605% | 68.714% | +1.109 pp |
| Matchup-flex JIT, going second | 36.960% | 51.470% | +14.510 pp | 63.865% | 67.437% | +3.572 pp | 75.583% | 77.077% | +1.494 pp |
| No discard control, going first | 19.740% | 25.259% | +5.519 pp | 56.829% | 60.275% | +3.446 pp | 73.585% | 75.722% | +2.137 pp |
| No discard control, going second | 40.232% | 59.291% | +19.059 pp | 68.436% | 73.785% | +5.349 pp | 79.981% | 82.360% | +2.379 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.204% ± 0.104 | 41.795% ± 0.156 | 59.098% ± 0.155 | 40.902% ± 0.155 |
| Matchup-flex JIT, going first | 16.917% ± 0.119 | 50.069% ± 0.158 | 67.605% ± 0.148 | 32.395% ± 0.148 |
| No discard control, going first | 19.740% ± 0.126 | 56.829% ± 0.157 | 73.585% ± 0.139 | 26.415% ± 0.139 |
| Strict JIT, going second | 29.752% ± 0.145 | 55.420% ± 0.157 | 67.361% ± 0.148 | 32.639% ± 0.148 |
| Matchup-flex JIT, going second | 36.960% ± 0.153 | 63.865% ± 0.152 | 75.583% ± 0.136 | 24.417% ± 0.136 |
| No discard control, going second | 40.232% ± 0.155 | 68.436% ± 0.147 | 79.981% ± 0.127 | 20.019% ± 0.127 |
| Strict JIT, turn-two Item lock, first | 4.493% ± 0.066 | 10.340% ± 0.096 | 18.468% ± 0.123 | 81.532% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.500% ± 0.066 | 27.680% ± 0.141 | 42.653% ± 0.156 | 57.347% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% ± 0.028 | 4.558% ± 0.066 | 9.673% ± 0.093 | 90.327% ± 0.093 |
| Strict JIT, turn-two Item lock, second | 14.127% ± 0.110 | 28.377% ± 0.143 | 37.109% ± 0.153 | 62.891% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.404% ± 0.123 | 37.103% ± 0.153 | 49.311% ± 0.158 | 50.689% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.830% ± 0.061 | 14.505% ± 0.111 | 20.172% ± 0.127 | 79.828% ± 0.127 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.118% ± 0.113 | 22.144% ± 0.131 | 77.856% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.010% ± 0.086 | 19.379% ± 0.125 | 25.885% ± 0.139 | 74.115% ± 0.139 |
| Garbodor + Boost Shake Ability lock, first | 5.581% ± 0.073 | 26.966% ± 0.140 | 40.894% ± 0.155 | 59.106% ± 0.155 |
| Garbodor + Boost Shake Ability lock, second | 17.453% ± 0.120 | 34.881% ± 0.151 | 46.594% ± 0.158 | 53.406% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.204% | 29.591% | 17.303% | 10.932% |
| Matchup-flex JIT, going first | 16.917% | 33.152% | 17.536% | 10.228% |
| No discard control, going first | 19.740% | 37.089% | 16.756% | 9.045% |
| Strict JIT, going second | 29.752% | 25.668% | 11.941% | 8.033% |
| Matchup-flex JIT, going second | 36.960% | 26.905% | 11.718% | 7.066% |
| No discard control, going second | 40.232% | 28.204% | 11.545% | 6.384% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.618% ± 0.126 | 49.028% ± 0.158 | 66.763% ± 0.149 | 33.237% ± 0.149 |
| Matchup-flex JIT, going first | 21.175% ± 0.129 | 50.717% ± 0.158 | 68.714% ± 0.147 | 31.286% ± 0.147 |
| No discard control, going first | 25.259% ± 0.137 | 60.275% ± 0.155 | 75.722% ± 0.136 | 24.278% ± 0.136 |
| Strict JIT, going second | 48.445% ± 0.158 | 64.490% ± 0.151 | 74.933% ± 0.137 | 25.067% ± 0.137 |
| Matchup-flex JIT, going second | 51.470% ± 0.158 | 67.437% ± 0.148 | 77.077% ± 0.133 | 22.923% ± 0.133 |
| No discard control, going second | 59.291% ± 0.155 | 73.785% ± 0.139 | 82.360% ± 0.121 | 17.640% ± 0.121 |
| Strict JIT, turn-two Item lock, first | 4.655% ± 0.067 | 8.659% ± 0.089 | 14.927% ± 0.113 | 85.073% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.297% ± 0.071 | 19.698% ± 0.126 | 32.183% ± 0.148 | 67.817% ± 0.148 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.161% ± 0.034 | 2.649% ± 0.051 | 5.350% ± 0.071 | 94.650% ± 0.071 |
| Strict JIT, turn-two Item lock, second | 6.790% ± 0.080 | 12.788% ± 0.106 | 19.077% ± 0.124 | 80.923% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.254% ± 0.107 | 25.774% ± 0.138 | 35.852% ± 0.152 | 64.148% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.980% ± 0.044 | 4.641% ± 0.067 | 7.349% ± 0.083 | 92.651% ± 0.083 |
| Strict JIT, Supporter lock, first | 1.851% ± 0.043 | 7.503% ± 0.083 | 12.731% ± 0.105 | 87.269% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.515% ± 0.078 | 11.815% ± 0.102 | 17.105% ± 0.119 | 82.895% ± 0.119 |
| Garbodor + Boost Shake Ability lock, first | 6.997% ± 0.081 | 21.601% ± 0.130 | 33.952% ± 0.150 | 66.048% ± 0.150 |
| Garbodor + Boost Shake Ability lock, second | 13.009% ± 0.106 | 25.466% ± 0.138 | 35.428% ± 0.151 | 64.572% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.618% | 29.410% | 17.735% | 10.555% |
| Matchup-flex JIT, going first | 21.175% | 29.542% | 17.997% | 10.452% |
| No discard control, going first | 25.259% | 35.016% | 15.447% | 7.220% |
| Strict JIT, going second | 48.445% | 16.045% | 10.443% | 6.802% |
| Matchup-flex JIT, going second | 51.470% | 15.967% | 9.640% | 6.359% |
| No discard control, going second | 59.291% | 14.494% | 8.575% | 4.667% |

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
| Secret Box use | 64.318% |
| Exploding Energy use | 78.484% |
| Steven use | 35.749% |
| Star Alchemy use | 48.088% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.048 per game |
| Missing route axis | 0.772 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.275 per game |
| Steven banks | 0.301 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.396 |
| Pineco/Forretress line | 0.425 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.026 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.473 |
| Discard zone | 0.236 |
| Stranded hand zone | 0.191 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `2aad2ac31a8d79ffd0b5009a92c553498c3fde8aab0eedc51a364ac1e71df040`.

Comparison CSV SHA-256: `63da80c82337b2c05c942a645106db93d994998543ec6fd73bf19be2601c9de7`.
