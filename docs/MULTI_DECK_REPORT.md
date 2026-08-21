# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% | 19.635% | +7.458 pp | 41.820% | 49.078% | +7.258 pp | 59.369% | 66.903% | +7.534 pp |
| Strict JIT, going second | 29.856% | 48.495% | +18.639 pp | 55.696% | 64.370% | +8.674 pp | 67.607% | 74.565% | +6.958 pp |
| Matchup-flex JIT, going first | 17.216% | 21.159% | +3.943 pp | 50.825% | 51.232% | +0.407 pp | 68.234% | 69.265% | +1.031 pp |
| Matchup-flex JIT, going second | 37.499% | 51.702% | +14.203 pp | 64.390% | 67.788% | +3.398 pp | 76.023% | 77.474% | +1.451 pp |
| No discard control, going first | 19.740% | 25.271% | +5.531 pp | 56.829% | 60.276% | +3.447 pp | 73.585% | 75.715% | +2.130 pp |
| No discard control, going second | 40.448% | 59.294% | +18.846 pp | 68.575% | 73.788% | +5.213 pp | 80.147% | 82.363% | +2.216 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% ± 0.103 | 41.820% ± 0.156 | 59.369% ± 0.155 | 40.631% ± 0.155 |
| Matchup-flex JIT, going first | 17.216% ± 0.119 | 50.825% ± 0.158 | 68.234% ± 0.147 | 31.766% ± 0.147 |
| No discard control, going first | 19.740% ± 0.126 | 56.829% ± 0.157 | 73.585% ± 0.139 | 26.415% ± 0.139 |
| Strict JIT, going second | 29.856% ± 0.145 | 55.696% ± 0.157 | 67.607% ± 0.148 | 32.393% ± 0.148 |
| Matchup-flex JIT, going second | 37.499% ± 0.153 | 64.390% ± 0.151 | 76.023% ± 0.135 | 23.977% ± 0.135 |
| No discard control, going second | 40.448% ± 0.155 | 68.575% ± 0.147 | 80.147% ± 0.126 | 19.853% ± 0.126 |
| Strict JIT, turn-two Item lock, first | 4.523% ± 0.066 | 10.518% ± 0.097 | 18.591% ± 0.123 | 81.409% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.499% ± 0.066 | 28.053% ± 0.142 | 43.019% ± 0.157 | 56.981% ± 0.157 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% ± 0.028 | 4.667% ± 0.067 | 9.645% ± 0.093 | 90.355% ± 0.093 |
| Strict JIT, turn-two Item lock, second | 14.077% ± 0.110 | 28.453% ± 0.143 | 37.255% ± 0.153 | 62.745% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.540% ± 0.123 | 37.310% ± 0.153 | 49.713% ± 0.158 | 50.287% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.874% ± 0.061 | 14.683% ± 0.112 | 20.372% ± 0.127 | 79.628% ± 0.127 |
| Strict JIT, Supporter lock, first | 0.005% ± 0.002 | 16.425% ± 0.117 | 23.367% ± 0.134 | 76.633% ± 0.134 |
| Strict JIT, Supporter lock, second | 7.918% ± 0.085 | 20.453% ± 0.128 | 26.831% ± 0.140 | 73.169% ± 0.140 |
| Garbodor + Boost Shake Ability lock, first | 5.660% ± 0.073 | 25.146% ± 0.137 | 37.724% ± 0.153 | 62.276% ± 0.153 |
| Garbodor + Boost Shake Ability lock, second | 15.438% ± 0.114 | 30.684% ± 0.146 | 41.874% ± 0.156 | 58.126% ± 0.156 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% | 29.643% | 17.549% | 10.887% |
| Matchup-flex JIT, going first | 17.216% | 33.609% | 17.409% | 10.025% |
| No discard control, going first | 19.740% | 37.089% | 16.756% | 9.045% |
| Strict JIT, going second | 29.856% | 25.840% | 11.911% | 7.918% |
| Matchup-flex JIT, going second | 37.499% | 26.891% | 11.633% | 6.930% |
| No discard control, going second | 40.448% | 28.127% | 11.572% | 6.444% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.635% ± 0.126 | 49.078% ± 0.158 | 66.903% ± 0.149 | 33.097% ± 0.149 |
| Matchup-flex JIT, going first | 21.159% ± 0.129 | 51.232% ± 0.158 | 69.265% ± 0.146 | 30.735% ± 0.146 |
| No discard control, going first | 25.271% ± 0.137 | 60.276% ± 0.155 | 75.715% ± 0.136 | 24.285% ± 0.136 |
| Strict JIT, going second | 48.495% ± 0.158 | 64.370% ± 0.151 | 74.565% ± 0.138 | 25.435% ± 0.138 |
| Matchup-flex JIT, going second | 51.702% ± 0.158 | 67.788% ± 0.148 | 77.474% ± 0.132 | 22.526% ± 0.132 |
| No discard control, going second | 59.294% ± 0.155 | 73.788% ± 0.139 | 82.363% ± 0.121 | 17.637% ± 0.121 |
| Strict JIT, turn-two Item lock, first | 4.655% ± 0.067 | 8.643% ± 0.089 | 14.924% ± 0.113 | 85.076% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.275% ± 0.071 | 19.733% ± 0.126 | 32.268% ± 0.148 | 67.732% ± 0.148 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.177% ± 0.034 | 2.646% ± 0.051 | 5.329% ± 0.071 | 94.671% ± 0.071 |
| Strict JIT, turn-two Item lock, second | 6.797% ± 0.080 | 12.785% ± 0.106 | 19.062% ± 0.124 | 80.938% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.348% ± 0.108 | 25.947% ± 0.139 | 36.063% ± 0.152 | 63.937% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.975% ± 0.044 | 4.635% ± 0.066 | 7.305% ± 0.082 | 92.695% ± 0.082 |
| Strict JIT, Supporter lock, first | 1.868% ± 0.043 | 8.706% ± 0.089 | 13.991% ± 0.110 | 86.009% ± 0.110 |
| Strict JIT, Supporter lock, second | 6.449% ± 0.078 | 12.474% ± 0.104 | 17.839% ± 0.121 | 82.161% ± 0.121 |
| Garbodor + Boost Shake Ability lock, first | 6.903% ± 0.080 | 21.036% ± 0.129 | 33.660% ± 0.149 | 66.340% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 13.027% ± 0.106 | 25.235% ± 0.137 | 35.276% ± 0.151 | 64.724% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.635% | 29.443% | 17.825% | 10.510% |
| Matchup-flex JIT, going first | 21.159% | 30.073% | 18.033% | 10.096% |
| No discard control, going first | 25.271% | 35.005% | 15.439% | 7.223% |
| Strict JIT, going second | 48.495% | 15.875% | 10.195% | 6.748% |
| Matchup-flex JIT, going second | 51.702% | 16.086% | 9.686% | 6.329% |
| No discard control, going second | 59.294% | 14.494% | 8.575% | 4.667% |

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
| Secret Box use | 64.320% |
| Exploding Energy use | 78.489% |
| Steven use | 35.739% |
| Star Alchemy use | 48.097% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.049 per game |
| Missing route axis | 0.772 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
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

Simulator policy digest: `51b2e36ce68d163b4b325794a55ce5bf669bbd7b42f127e85e186d56cf74af29`.

Comparison CSV SHA-256: `52d38f4cafe12727874247c3263520c4d5ab1aaedb3711cc5ea1749ff647e581`.
