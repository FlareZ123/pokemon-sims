# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.160% | 19.762% | +7.602 pp | 41.210% | 49.001% | +7.791 pp | 58.069% | 66.554% | +8.485 pp |
| Strict JIT, going second | 29.748% | 48.305% | +18.557 pp | 54.406% | 64.267% | +9.861 pp | 65.856% | 74.630% | +8.774 pp |
| Matchup-flex JIT, going first | 16.529% | 21.284% | +4.755 pp | 49.754% | 50.380% | +0.626 pp | 65.676% | 68.485% | +2.809 pp |
| Matchup-flex JIT, going second | 36.989% | 51.349% | +14.360 pp | 62.061% | 67.218% | +5.157 pp | 72.686% | 76.804% | +4.118 pp |
| No discard control, going first | 19.958% | 25.216% | +5.258 pp | 56.020% | 60.418% | +4.398 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.122% | 73.939% | +6.817 pp | 78.369% | 82.494% | +4.125 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.160% ± 0.103 | 41.210% ± 0.156 | 58.069% ± 0.156 | 41.931% ± 0.156 |
| Matchup-flex JIT, going first | 16.529% ± 0.117 | 49.754% ± 0.158 | 65.676% ± 0.150 | 34.324% ± 0.150 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.588% ± 0.066 | 10.403% ± 0.097 | 18.291% ± 0.122 | 81.709% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.431% ± 0.065 | 26.771% ± 0.140 | 40.436% ± 0.155 | 59.564% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.814% ± 0.028 | 4.665% ± 0.067 | 9.585% ± 0.093 | 90.415% ± 0.093 |
| Strict JIT, going second | 29.748% ± 0.145 | 54.406% ± 0.157 | 65.856% ± 0.150 | 34.144% ± 0.150 |
| Matchup-flex JIT, going second | 36.989% ± 0.153 | 62.061% ± 0.153 | 72.686% ± 0.141 | 27.314% ± 0.141 |
| No discard control, going second | 39.964% ± 0.155 | 67.122% ± 0.149 | 78.369% ± 0.130 | 21.631% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.250% ± 0.111 | 28.427% ± 0.143 | 36.938% ± 0.153 | 63.062% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.542% ± 0.123 | 35.985% ± 0.152 | 46.825% ± 0.158 | 53.175% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.797% ± 0.060 | 14.322% ± 0.111 | 19.877% ± 0.126 | 80.123% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 14.705% ± 0.112 | 21.132% ± 0.129 | 78.868% ± 0.129 |
| Strict JIT, Supporter lock, second | 8.037% ± 0.086 | 19.287% ± 0.125 | 25.213% ± 0.137 | 74.787% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.160% | 29.050% | 16.859% | 10.502% |
| Matchup-flex JIT, going first | 16.529% | 33.225% | 15.922% | 9.651% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.748% | 24.658% | 11.450% | 7.901% |
| Matchup-flex JIT, going second | 36.989% | 25.072% | 10.625% | 7.143% |
| No discard control, going second | 39.964% | 27.158% | 11.247% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.762% ± 0.126 | 49.001% ± 0.158 | 66.554% ± 0.149 | 33.446% ± 0.149 |
| Matchup-flex JIT, going first | 21.284% ± 0.129 | 50.380% ± 0.158 | 68.485% ± 0.147 | 31.515% ± 0.147 |
| No discard control, going first | 25.216% ± 0.137 | 60.418% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.656% ± 0.067 | 8.665% ± 0.089 | 14.898% ± 0.113 | 85.102% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.922% ± 0.068 | 18.591% ± 0.123 | 30.594% ± 0.146 | 69.406% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.973% ± 0.031 | 2.399% ± 0.048 | 4.953% ± 0.069 | 95.047% ± 0.069 |
| Strict JIT, going second | 48.305% ± 0.158 | 64.267% ± 0.152 | 74.630% ± 0.138 | 25.370% ± 0.138 |
| Matchup-flex JIT, going second | 51.349% ± 0.158 | 67.218% ± 0.148 | 76.804% ± 0.133 | 23.196% ± 0.133 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.866% ± 0.080 | 12.754% ± 0.105 | 19.120% ± 0.124 | 80.880% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.506% ± 0.105 | 24.644% ± 0.136 | 34.460% ± 0.150 | 65.540% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.903% ± 0.043 | 4.399% ± 0.065 | 6.943% ± 0.080 | 93.057% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.361% ± 0.083 | 12.495% ± 0.105 | 87.505% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.551% ± 0.078 | 11.663% ± 0.102 | 16.865% ± 0.118 | 83.135% ± 0.118 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.762% | 29.239% | 17.553% | 10.433% |
| Matchup-flex JIT, going first | 21.284% | 29.096% | 18.105% | 10.423% |
| No discard control, going first | 25.216% | 35.202% | 15.387% | 7.192% |
| Strict JIT, going second | 48.305% | 15.962% | 10.363% | 6.724% |
| Matchup-flex JIT, going second | 51.349% | 15.869% | 9.586% | 6.433% |
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

Simulator policy digest: `bb0f5009c57945c546e7eb17aef13ca46940195ed93fd7b80e936561be057572`.

Comparison CSV SHA-256: `3405340913f6561a26b6c9e978cd2f326f08c2b065a7cda0c416c20b85d0aaa6`.
