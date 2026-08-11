# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.259% | 19.681% | +7.422 pp | 41.802% | 49.103% | +7.301 pp | 59.295% | 66.562% | +7.267 pp |
| Strict JIT, going second | 29.649% | 48.206% | +18.557 pp | 54.855% | 63.990% | +9.135 pp | 66.972% | 74.459% | +7.487 pp |
| Matchup-flex JIT, going first | 16.825% | 21.136% | +4.311 pp | 50.072% | 50.632% | +0.560 pp | 67.062% | 68.660% | +1.598 pp |
| Matchup-flex JIT, going second | 37.282% | 51.515% | +14.233 pp | 63.021% | 67.142% | +4.121 pp | 74.512% | 76.863% | +2.351 pp |
| No discard control, going first | 19.637% | 25.259% | +5.622 pp | 56.790% | 60.275% | +3.485 pp | 73.566% | 75.722% | +2.156 pp |
| No discard control, going second | 40.376% | 59.291% | +18.915 pp | 68.538% | 73.785% | +5.247 pp | 80.045% | 82.360% | +2.315 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.259% ± 0.104 | 41.802% ± 0.156 | 59.295% ± 0.155 | 40.705% ± 0.155 |
| Matchup-flex JIT, going first | 16.825% ± 0.118 | 50.072% ± 0.158 | 67.062% ± 0.149 | 32.938% ± 0.149 |
| No discard control, going first | 19.637% ± 0.126 | 56.790% ± 0.157 | 73.566% ± 0.139 | 26.434% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.554% ± 0.066 | 10.391% ± 0.096 | 18.394% ± 0.123 | 81.606% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.483% ± 0.065 | 27.170% ± 0.141 | 41.526% ± 0.156 | 58.474% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.767% ± 0.028 | 4.619% ± 0.066 | 9.581% ± 0.093 | 90.419% ± 0.093 |
| Strict JIT, going second | 29.649% ± 0.144 | 54.855% ± 0.157 | 66.972% ± 0.149 | 33.028% ± 0.149 |
| Matchup-flex JIT, going second | 37.282% ± 0.153 | 63.021% ± 0.153 | 74.512% ± 0.138 | 25.488% ± 0.138 |
| No discard control, going second | 40.376% ± 0.155 | 68.538% ± 0.147 | 80.045% ± 0.126 | 19.955% ± 0.126 |
| Strict JIT, turn-two Item lock, second | 14.083% ± 0.110 | 28.321% ± 0.142 | 37.012% ± 0.153 | 62.988% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.446% ± 0.123 | 36.648% ± 0.152 | 48.384% ± 0.158 | 51.616% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.801% ± 0.060 | 14.427% ± 0.111 | 19.893% ± 0.126 | 80.107% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.001% ± 0.001 | 14.769% ± 0.112 | 21.486% ± 0.130 | 78.514% ± 0.130 |
| Strict JIT, Supporter lock, second | 7.971% ± 0.086 | 19.169% ± 0.124 | 25.417% ± 0.138 | 74.583% ± 0.138 |
| Garbodor + Boost Shake Ability lock, first | 5.566% ± 0.072 | 26.987% ± 0.140 | 40.741% ± 0.155 | 59.259% ± 0.155 |
| Garbodor + Boost Shake Ability lock, second | 17.283% ± 0.120 | 34.427% ± 0.150 | 46.174% ± 0.158 | 53.826% ± 0.158 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.259% | 29.543% | 17.493% | 10.880% |
| Matchup-flex JIT, going first | 16.825% | 33.247% | 16.990% | 9.935% |
| No discard control, going first | 19.637% | 37.153% | 16.776% | 9.012% |
| Strict JIT, going second | 29.649% | 25.206% | 12.117% | 8.004% |
| Matchup-flex JIT, going second | 37.282% | 25.739% | 11.491% | 7.218% |
| No discard control, going second | 40.376% | 28.162% | 11.507% | 6.391% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.681% ± 0.126 | 49.103% ± 0.158 | 66.562% ± 0.149 | 33.438% ± 0.149 |
| Matchup-flex JIT, going first | 21.136% ± 0.129 | 50.632% ± 0.158 | 68.660% ± 0.147 | 31.340% ± 0.147 |
| No discard control, going first | 25.259% ± 0.137 | 60.275% ± 0.155 | 75.722% ± 0.136 | 24.278% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.674% ± 0.067 | 8.639% ± 0.089 | 14.877% ± 0.113 | 85.123% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.112% ± 0.070 | 18.411% ± 0.123 | 30.454% ± 0.146 | 69.546% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.194% ± 0.034 | 2.568% ± 0.050 | 4.996% ± 0.069 | 95.004% ± 0.069 |
| Strict JIT, going second | 48.206% ± 0.158 | 63.990% ± 0.152 | 74.459% ± 0.138 | 25.541% ± 0.138 |
| Matchup-flex JIT, going second | 51.515% ± 0.158 | 67.142% ± 0.149 | 76.863% ± 0.133 | 23.137% ± 0.133 |
| No discard control, going second | 59.291% ± 0.155 | 73.785% ± 0.139 | 82.360% ± 0.121 | 17.640% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.869% ± 0.080 | 12.825% ± 0.106 | 19.154% ± 0.124 | 80.846% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.620% ± 0.105 | 24.701% ± 0.136 | 34.522% ± 0.150 | 65.478% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.896% ± 0.043 | 4.364% ± 0.065 | 6.915% ± 0.080 | 93.085% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.361% ± 0.083 | 12.495% ± 0.105 | 87.505% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.551% ± 0.078 | 11.663% ± 0.102 | 16.865% ± 0.118 | 83.135% ± 0.118 |
| Garbodor + Boost Shake Ability lock, first | 6.975% ± 0.081 | 21.440% ± 0.130 | 33.644% ± 0.149 | 66.356% ± 0.149 |
| Garbodor + Boost Shake Ability lock, second | 12.814% ± 0.106 | 25.377% ± 0.138 | 35.274% ± 0.151 | 64.726% ± 0.151 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.681% | 29.422% | 17.459% | 10.497% |
| Matchup-flex JIT, going first | 21.136% | 29.496% | 18.028% | 10.286% |
| No discard control, going first | 25.259% | 35.016% | 15.447% | 7.220% |
| Strict JIT, going second | 48.206% | 15.784% | 10.469% | 6.731% |
| Matchup-flex JIT, going second | 51.515% | 15.627% | 9.721% | 6.263% |
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

Simulator policy digest: `454996282c216f64698bd551aee616b60959e05bdf7df178dc2f5c5238f83ee4`.

Comparison CSV SHA-256: `4c91ade612736b078dd21856c27b8958c5c1a765b28e4d4b8640a28988798896`.
