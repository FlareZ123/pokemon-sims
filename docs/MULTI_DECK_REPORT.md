# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% | 19.554% | +7.396 pp | 41.807% | 48.970% | +7.163 pp | 59.222% | 66.762% | +7.540 pp |
| Strict JIT, going second | 29.718% | 48.516% | +18.798 pp | 55.558% | 64.495% | +8.937 pp | 67.532% | 74.919% | +7.387 pp |
| Matchup-flex JIT, going first | 17.232% | 21.321% | +4.089 pp | 50.806% | 51.203% | +0.397 pp | 68.194% | 69.208% | +1.014 pp |
| Matchup-flex JIT, going second | 37.286% | 51.672% | +14.386 pp | 64.176% | 67.753% | +3.577 pp | 75.820% | 77.242% | +1.422 pp |
| No discard control, going first | 19.740% | 25.271% | +5.531 pp | 56.829% | 60.276% | +3.447 pp | 73.585% | 75.715% | +2.130 pp |
| No discard control, going second | 40.232% | 59.294% | +19.062 pp | 68.436% | 73.788% | +5.352 pp | 79.981% | 82.363% | +2.382 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% ± 0.103 | 41.807% ± 0.156 | 59.222% ± 0.155 | 40.778% ± 0.155 |
| Matchup-flex JIT, going first | 17.232% ± 0.119 | 50.806% ± 0.158 | 68.194% ± 0.147 | 31.806% ± 0.147 |
| No discard control, going first | 19.740% ± 0.126 | 56.829% ± 0.157 | 73.585% ± 0.139 | 26.415% ± 0.139 |
| Strict JIT, going second | 29.718% ± 0.145 | 55.558% ± 0.157 | 67.532% ± 0.148 | 32.468% ± 0.148 |
| Matchup-flex JIT, going second | 37.286% ± 0.153 | 64.176% ± 0.152 | 75.820% ± 0.135 | 24.180% ± 0.135 |
| No discard control, going second | 40.232% ± 0.155 | 68.436% ± 0.147 | 79.981% ± 0.127 | 20.019% ± 0.127 |
| Strict JIT, turn-two Item lock, first | 4.493% ± 0.066 | 10.340% ± 0.096 | 18.468% ± 0.123 | 81.532% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.531% ± 0.066 | 27.748% ± 0.142 | 42.666% ± 0.156 | 57.334% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.765% ± 0.028 | 4.610% ± 0.066 | 9.628% ± 0.093 | 90.372% ± 0.093 |
| Strict JIT, turn-two Item lock, second | 14.127% ± 0.110 | 28.377% ± 0.143 | 37.109% ± 0.153 | 62.891% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.467% ± 0.123 | 37.137% ± 0.153 | 49.356% ± 0.158 | 50.644% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.900% ± 0.061 | 14.510% ± 0.111 | 20.154% ± 0.127 | 79.846% ± 0.127 |
| Strict JIT, Supporter lock, first | 0.005% ± 0.002 | 16.426% ± 0.117 | 23.376% ± 0.134 | 76.624% ± 0.134 |
| Strict JIT, Supporter lock, second | 7.850% ± 0.085 | 20.357% ± 0.127 | 26.735% ± 0.140 | 73.265% ± 0.140 |
| Garbodor + Boost Shake Ability lock, first | 5.687% ± 0.073 | 26.946% ± 0.140 | 41.056% ± 0.156 | 58.944% ± 0.156 |
| Garbodor + Boost Shake Ability lock, second | 17.437% ± 0.120 | 34.720% ± 0.151 | 46.467% ± 0.158 | 53.533% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% | 29.649% | 17.415% | 10.940% |
| Matchup-flex JIT, going first | 17.232% | 33.574% | 17.388% | 10.013% |
| No discard control, going first | 19.740% | 37.089% | 16.756% | 9.045% |
| Strict JIT, going second | 29.718% | 25.840% | 11.974% | 7.946% |
| Matchup-flex JIT, going second | 37.286% | 26.890% | 11.644% | 6.917% |
| No discard control, going second | 40.232% | 28.204% | 11.545% | 6.384% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.554% ± 0.125 | 48.970% ± 0.158 | 66.762% ± 0.149 | 33.238% ± 0.149 |
| Matchup-flex JIT, going first | 21.321% ± 0.130 | 51.203% ± 0.158 | 69.208% ± 0.146 | 30.792% ± 0.146 |
| No discard control, going first | 25.271% ± 0.137 | 60.276% ± 0.155 | 75.715% ± 0.136 | 24.285% ± 0.136 |
| Strict JIT, going second | 48.516% ± 0.158 | 64.495% ± 0.151 | 74.919% ± 0.137 | 25.081% ± 0.137 |
| Matchup-flex JIT, going second | 51.672% ± 0.158 | 67.753% ± 0.148 | 77.242% ± 0.133 | 22.758% ± 0.133 |
| No discard control, going second | 59.294% ± 0.155 | 73.788% ± 0.139 | 82.363% ± 0.121 | 17.637% ± 0.121 |
| Strict JIT, turn-two Item lock, first | 4.655% ± 0.067 | 8.659% ± 0.089 | 14.927% ± 0.113 | 85.073% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.282% ± 0.071 | 19.760% ± 0.126 | 32.292% ± 0.148 | 67.708% ± 0.148 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.178% ± 0.034 | 2.643% ± 0.051 | 5.322% ± 0.071 | 94.678% ± 0.071 |
| Strict JIT, turn-two Item lock, second | 6.790% ± 0.080 | 12.788% ± 0.106 | 19.077% ± 0.124 | 80.923% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.288% ± 0.107 | 25.767% ± 0.138 | 35.820% ± 0.152 | 64.180% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.985% ± 0.044 | 4.635% ± 0.066 | 7.286% ± 0.082 | 92.714% ± 0.082 |
| Strict JIT, Supporter lock, first | 1.851% ± 0.043 | 8.681% ± 0.089 | 13.975% ± 0.110 | 86.025% ± 0.110 |
| Strict JIT, Supporter lock, second | 6.430% ± 0.078 | 12.483% ± 0.105 | 17.879% ± 0.121 | 82.121% ± 0.121 |
| Garbodor + Boost Shake Ability lock, first | 6.979% ± 0.081 | 21.091% ± 0.129 | 33.742% ± 0.150 | 66.258% ± 0.150 |
| Garbodor + Boost Shake Ability lock, second | 12.960% ± 0.106 | 25.188% ± 0.137 | 35.258% ± 0.151 | 64.742% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.554% | 29.416% | 17.792% | 10.534% |
| Matchup-flex JIT, going first | 21.321% | 29.882% | 18.005% | 10.121% |
| No discard control, going first | 25.271% | 35.005% | 15.439% | 7.223% |
| Strict JIT, going second | 48.516% | 15.979% | 10.424% | 6.781% |
| Matchup-flex JIT, going second | 51.672% | 16.081% | 9.489% | 6.222% |
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

Simulator policy digest: `7b374a5ec03ef53ee7d0ae31524dfb0dfda50e35f9a3665bfc771377b95a9c70`.

Comparison CSV SHA-256: `2dd44564a3d835473fec93166608e8c96c6dc5a18b78a6761773e2ba79974d12`.
