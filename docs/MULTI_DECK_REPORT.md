# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.165% | 19.636% | +7.471 pp | 41.799% | 49.079% | +7.280 pp | 59.361% | 66.906% | +7.545 pp |
| Strict JIT, going second | 29.871% | 48.477% | +18.606 pp | 55.700% | 64.362% | +8.662 pp | 67.612% | 74.553% | +6.941 pp |
| Matchup-flex JIT, going first | 17.210% | 21.158% | +3.948 pp | 50.824% | 51.223% | +0.399 pp | 68.232% | 69.262% | +1.030 pp |
| Matchup-flex JIT, going second | 37.501% | 51.721% | +14.220 pp | 64.392% | 67.789% | +3.397 pp | 76.025% | 77.476% | +1.451 pp |
| No discard control, going first | 19.740% | 25.271% | +5.531 pp | 56.829% | 60.276% | +3.447 pp | 73.585% | 75.715% | +2.130 pp |
| No discard control, going second | 40.448% | 59.294% | +18.846 pp | 68.575% | 73.788% | +5.213 pp | 80.147% | 82.363% | +2.216 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.165% ± 0.103 | 41.799% ± 0.156 | 59.361% ± 0.155 | 40.639% ± 0.155 |
| Matchup-flex JIT, going first | 17.210% ± 0.119 | 50.824% ± 0.158 | 68.232% ± 0.147 | 31.768% ± 0.147 |
| No discard control, going first | 19.740% ± 0.126 | 56.829% ± 0.157 | 73.585% ± 0.139 | 26.415% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.523% ± 0.066 | 10.518% ± 0.097 | 18.591% ± 0.123 | 81.409% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.498% ± 0.066 | 28.043% ± 0.142 | 43.015% ± 0.157 | 56.985% ± 0.157 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% ± 0.028 | 4.667% ± 0.067 | 9.645% ± 0.093 | 90.355% ± 0.093 |
| Strict JIT, going second | 29.871% ± 0.145 | 55.700% ± 0.157 | 67.612% ± 0.148 | 32.388% ± 0.148 |
| Matchup-flex JIT, going second | 37.501% ± 0.153 | 64.392% ± 0.151 | 76.025% ± 0.135 | 23.975% ± 0.135 |
| No discard control, going second | 40.448% ± 0.155 | 68.575% ± 0.147 | 80.147% ± 0.126 | 19.853% ± 0.126 |
| Strict JIT, turn-two Item lock, second | 14.077% ± 0.110 | 28.453% ± 0.143 | 37.255% ± 0.153 | 62.745% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.523% ± 0.123 | 37.296% ± 0.153 | 49.707% ± 0.158 | 50.293% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.874% ± 0.061 | 14.683% ± 0.112 | 20.372% ± 0.127 | 79.628% ± 0.127 |
| Strict JIT, Supporter lock, first | 0.005% ± 0.002 | 16.425% ± 0.117 | 23.367% ± 0.134 | 76.633% ± 0.134 |
| Strict JIT, Supporter lock, second | 7.918% ± 0.085 | 20.447% ± 0.128 | 26.828% ± 0.140 | 73.172% ± 0.140 |
| Garbodor + Boost Shake Ability lock, first | 5.660% ± 0.073 | 25.146% ± 0.137 | 37.724% ± 0.153 | 62.276% ± 0.153 |
| Garbodor + Boost Shake Ability lock, second | 15.437% ± 0.114 | 30.692% ± 0.146 | 41.876% ± 0.156 | 58.124% ± 0.156 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.165% | 29.634% | 17.562% | 10.891% |
| Matchup-flex JIT, going first | 17.210% | 33.614% | 17.408% | 10.027% |
| No discard control, going first | 19.740% | 37.089% | 16.756% | 9.045% |
| Strict JIT, going second | 29.871% | 25.829% | 11.912% | 7.918% |
| Matchup-flex JIT, going second | 37.501% | 26.891% | 11.633% | 6.929% |
| No discard control, going second | 40.448% | 28.127% | 11.572% | 6.444% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.636% ± 0.126 | 49.079% ± 0.158 | 66.906% ± 0.149 | 33.094% ± 0.149 |
| Matchup-flex JIT, going first | 21.158% ± 0.129 | 51.223% ± 0.158 | 69.262% ± 0.146 | 30.738% ± 0.146 |
| No discard control, going first | 25.271% ± 0.137 | 60.276% ± 0.155 | 75.715% ± 0.136 | 24.285% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.655% ± 0.067 | 8.643% ± 0.089 | 14.924% ± 0.113 | 85.076% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.272% ± 0.071 | 19.743% ± 0.126 | 32.278% ± 0.148 | 67.722% ± 0.148 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.177% ± 0.034 | 2.646% ± 0.051 | 5.329% ± 0.071 | 94.671% ± 0.071 |
| Strict JIT, going second | 48.477% ± 0.158 | 64.362% ± 0.151 | 74.553% ± 0.138 | 25.447% ± 0.138 |
| Matchup-flex JIT, going second | 51.721% ± 0.158 | 67.789% ± 0.148 | 77.476% ± 0.132 | 22.524% ± 0.132 |
| No discard control, going second | 59.294% ± 0.155 | 73.788% ± 0.139 | 82.363% ± 0.121 | 17.637% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.797% ± 0.080 | 12.785% ± 0.106 | 19.062% ± 0.124 | 80.938% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.352% ± 0.108 | 25.943% ± 0.139 | 36.056% ± 0.152 | 63.944% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.975% ± 0.044 | 4.635% ± 0.066 | 7.305% ± 0.082 | 92.695% ± 0.082 |
| Strict JIT, Supporter lock, first | 1.869% ± 0.043 | 8.701% ± 0.089 | 13.993% ± 0.110 | 86.007% ± 0.110 |
| Strict JIT, Supporter lock, second | 6.436% ± 0.078 | 12.471% ± 0.104 | 17.851% ± 0.121 | 82.149% ± 0.121 |
| Garbodor + Boost Shake Ability lock, first | 6.903% ± 0.080 | 21.037% ± 0.129 | 33.656% ± 0.149 | 66.344% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 13.026% ± 0.106 | 25.234% ± 0.137 | 35.275% ± 0.151 | 64.725% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.636% | 29.443% | 17.827% | 10.509% |
| Matchup-flex JIT, going first | 21.158% | 30.065% | 18.039% | 10.102% |
| No discard control, going first | 25.271% | 35.005% | 15.439% | 7.223% |
| Strict JIT, going second | 48.477% | 15.885% | 10.191% | 6.748% |
| Matchup-flex JIT, going second | 51.721% | 16.068% | 9.687% | 6.323% |
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

Simulator policy digest: `edc996cdd312e0a50f09916aeab4c8d757041794b88fbac446b59921675415da`.

Comparison CSV SHA-256: `82ddaa253682e97c6b9bdbd88875b6b86fe4c792779874bde2f26424125c9aaf`.
