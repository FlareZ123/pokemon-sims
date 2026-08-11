# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% | 19.627% | +7.343 pp | 41.904% | 49.020% | +7.116 pp | 59.175% | 66.561% | +7.386 pp |
| Strict JIT, going second | 29.932% | 48.239% | +18.307 pp | 55.225% | 64.112% | +8.887 pp | 67.285% | 74.429% | +7.144 pp |
| Matchup-flex JIT, going first | 16.841% | 21.121% | +4.280 pp | 50.431% | 50.679% | +0.248 pp | 67.346% | 68.697% | +1.351 pp |
| Matchup-flex JIT, going second | 37.368% | 51.478% | +14.110 pp | 63.119% | 67.144% | +4.025 pp | 74.486% | 76.904% | +2.418 pp |
| No discard control, going first | 19.770% | 25.259% | +5.489 pp | 56.731% | 60.275% | +3.544 pp | 73.456% | 75.722% | +2.266 pp |
| No discard control, going second | 40.269% | 59.291% | +19.022 pp | 68.437% | 73.785% | +5.348 pp | 79.955% | 82.360% | +2.405 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% ± 0.104 | 41.904% ± 0.156 | 59.175% ± 0.155 | 40.825% ± 0.155 |
| Matchup-flex JIT, going first | 16.841% ± 0.118 | 50.431% ± 0.158 | 67.346% ± 0.148 | 32.654% ± 0.148 |
| No discard control, going first | 19.770% ± 0.126 | 56.731% ± 0.157 | 73.456% ± 0.140 | 26.544% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.548% ± 0.066 | 10.389% ± 0.096 | 18.380% ± 0.122 | 81.620% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.529% ± 0.066 | 27.129% ± 0.141 | 41.328% ± 0.156 | 58.672% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.770% ± 0.028 | 4.606% ± 0.066 | 9.569% ± 0.093 | 90.431% ± 0.093 |
| Strict JIT, going second | 29.932% ± 0.145 | 55.225% ± 0.157 | 67.285% ± 0.148 | 32.715% ± 0.148 |
| Matchup-flex JIT, going second | 37.368% ± 0.153 | 63.119% ± 0.153 | 74.486% ± 0.138 | 25.514% ± 0.138 |
| No discard control, going second | 40.269% ± 0.155 | 68.437% ± 0.147 | 79.955% ± 0.127 | 20.045% ± 0.127 |
| Strict JIT, turn-two Item lock, second | 14.056% ± 0.110 | 28.291% ± 0.142 | 36.990% ± 0.153 | 63.010% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.383% ± 0.122 | 36.650% ± 0.152 | 48.384% ± 0.158 | 51.616% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.798% ± 0.060 | 14.428% ± 0.111 | 19.916% ± 0.126 | 80.084% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 15.020% ± 0.113 | 22.025% ± 0.131 | 77.975% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.011% ± 0.086 | 19.400% ± 0.125 | 25.858% ± 0.138 | 74.142% ± 0.138 |
| Garbodor + Boost Shake Ability lock, first | 5.564% ± 0.072 | 26.973% ± 0.140 | 40.832% ± 0.155 | 59.168% ± 0.155 |
| Garbodor + Boost Shake Ability lock, second | 17.334% ± 0.120 | 34.484% ± 0.150 | 46.321% ± 0.158 | 53.679% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.284% | 29.620% | 17.271% | 10.894% |
| Matchup-flex JIT, going first | 16.841% | 33.590% | 16.915% | 9.829% |
| No discard control, going first | 19.770% | 36.961% | 16.725% | 9.058% |
| Strict JIT, going second | 29.932% | 25.293% | 12.060% | 7.942% |
| Matchup-flex JIT, going second | 37.368% | 25.751% | 11.367% | 7.162% |
| No discard control, going second | 40.269% | 28.168% | 11.518% | 6.385% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.627% ± 0.126 | 49.020% ± 0.158 | 66.561% ± 0.149 | 33.439% ± 0.149 |
| Matchup-flex JIT, going first | 21.121% ± 0.129 | 50.679% ± 0.158 | 68.697% ± 0.147 | 31.303% ± 0.147 |
| No discard control, going first | 25.259% ± 0.137 | 60.275% ± 0.155 | 75.722% ± 0.136 | 24.278% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.674% ± 0.067 | 8.639% ± 0.089 | 14.877% ± 0.113 | 85.123% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.077% ± 0.069 | 18.609% ± 0.123 | 30.878% ± 0.146 | 69.122% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.194% ± 0.034 | 2.568% ± 0.050 | 4.996% ± 0.069 | 95.004% ± 0.069 |
| Strict JIT, going second | 48.239% ± 0.158 | 64.112% ± 0.152 | 74.429% ± 0.138 | 25.571% ± 0.138 |
| Matchup-flex JIT, going second | 51.478% ± 0.158 | 67.144% ± 0.149 | 76.904% ± 0.133 | 23.096% ± 0.133 |
| No discard control, going second | 59.291% ± 0.155 | 73.785% ± 0.139 | 82.360% ± 0.121 | 17.640% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.869% ± 0.080 | 12.825% ± 0.106 | 19.154% ± 0.124 | 80.846% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.675% ± 0.105 | 24.800% ± 0.137 | 34.623% ± 0.150 | 65.377% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.896% ± 0.043 | 4.364% ± 0.065 | 6.915% ± 0.080 | 93.085% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.868% ± 0.043 | 7.515% ± 0.083 | 12.762% ± 0.106 | 87.238% ± 0.106 |
| Strict JIT, Supporter lock, second | 6.499% ± 0.078 | 11.749% ± 0.102 | 17.034% ± 0.119 | 82.966% ± 0.119 |
| Garbodor + Boost Shake Ability lock, first | 6.957% ± 0.080 | 21.509% ± 0.130 | 33.701% ± 0.149 | 66.299% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 12.856% ± 0.106 | 25.376% ± 0.138 | 35.351% ± 0.151 | 64.649% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.627% | 29.393% | 17.541% | 10.515% |
| Matchup-flex JIT, going first | 21.121% | 29.558% | 18.018% | 10.233% |
| No discard control, going first | 25.259% | 35.016% | 15.447% | 7.220% |
| Strict JIT, going second | 48.239% | 15.873% | 10.317% | 6.743% |
| Matchup-flex JIT, going second | 51.478% | 15.666% | 9.760% | 6.266% |
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

Simulator policy digest: `a6113cc7b9f798275f0a78624810343e70a2c5ab98704094e6dd35ba0175bfb6`.

Comparison CSV SHA-256: `451ab09d73a42c04441b796588ef92917553d63cbfbb2062ad0fe08739f8d985`.
