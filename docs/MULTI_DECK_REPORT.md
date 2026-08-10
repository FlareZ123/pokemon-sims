# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% | 19.769% | +7.544 pp | 41.285% | 49.038% | +7.753 pp | 58.225% | 66.573% | +8.348 pp |
| Strict JIT, going second | 29.794% | 48.312% | +18.518 pp | 54.679% | 64.266% | +9.587 pp | 66.283% | 74.635% | +8.352 pp |
| Matchup-flex JIT, going first | 16.933% | 21.357% | +4.424 pp | 49.809% | 50.548% | +0.739 pp | 65.876% | 68.381% | +2.505 pp |
| Matchup-flex JIT, going second | 37.093% | 51.433% | +14.340 pp | 62.069% | 67.199% | +5.130 pp | 72.892% | 76.756% | +3.864 pp |
| No discard control, going first | 19.958% | 25.207% | +5.249 pp | 56.020% | 60.410% | +4.390 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.122% | 73.939% | +6.817 pp | 78.369% | 82.494% | +4.125 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% ± 0.104 | 41.285% ± 0.156 | 58.225% ± 0.156 | 41.775% ± 0.156 |
| Matchup-flex JIT, going first | 16.933% ± 0.119 | 49.809% ± 0.158 | 65.876% ± 0.150 | 34.124% ± 0.150 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.625% ± 0.066 | 10.455% ± 0.097 | 18.344% ± 0.122 | 81.656% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.449% ± 0.065 | 26.837% ± 0.140 | 40.528% ± 0.155 | 59.472% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.814% ± 0.028 | 4.665% ± 0.067 | 9.585% ± 0.093 | 90.415% ± 0.093 |
| Strict JIT, going second | 29.794% ± 0.145 | 54.679% ± 0.157 | 66.283% ± 0.149 | 33.717% ± 0.149 |
| Matchup-flex JIT, going second | 37.093% ± 0.153 | 62.069% ± 0.153 | 72.892% ± 0.141 | 27.108% ± 0.141 |
| No discard control, going second | 39.964% ± 0.155 | 67.122% ± 0.149 | 78.369% ± 0.130 | 21.631% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.233% ± 0.110 | 28.553% ± 0.143 | 37.057% ± 0.153 | 62.943% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.502% ± 0.123 | 35.938% ± 0.152 | 46.817% ± 0.158 | 53.183% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.797% ± 0.060 | 14.322% ± 0.111 | 19.877% ± 0.126 | 80.123% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 14.719% ± 0.112 | 21.198% ± 0.129 | 78.802% ± 0.129 |
| Strict JIT, Supporter lock, second | 8.048% ± 0.086 | 19.319% ± 0.125 | 25.262% ± 0.137 | 74.738% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% | 29.060% | 16.940% | 10.544% |
| Matchup-flex JIT, going first | 16.933% | 32.876% | 16.067% | 9.598% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.794% | 24.885% | 11.604% | 7.799% |
| Matchup-flex JIT, going second | 37.093% | 24.976% | 10.823% | 7.152% |
| No discard control, going second | 39.964% | 27.158% | 11.247% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.769% ± 0.126 | 49.038% ± 0.158 | 66.573% ± 0.149 | 33.427% ± 0.149 |
| Matchup-flex JIT, going first | 21.357% ± 0.130 | 50.548% ± 0.158 | 68.381% ± 0.147 | 31.619% ± 0.147 |
| No discard control, going first | 25.207% ± 0.137 | 60.410% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.656% ± 0.067 | 8.665% ± 0.089 | 14.898% ± 0.113 | 85.102% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.922% ± 0.068 | 18.591% ± 0.123 | 30.594% ± 0.146 | 69.406% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.973% ± 0.031 | 2.399% ± 0.048 | 4.953% ± 0.069 | 95.047% ± 0.069 |
| Strict JIT, going second | 48.312% ± 0.158 | 64.266% ± 0.152 | 74.635% ± 0.138 | 25.365% ± 0.138 |
| Matchup-flex JIT, going second | 51.433% ± 0.158 | 67.199% ± 0.148 | 76.756% ± 0.134 | 23.244% ± 0.134 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.866% ± 0.080 | 12.754% ± 0.105 | 19.120% ± 0.124 | 80.880% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.506% ± 0.105 | 24.644% ± 0.136 | 34.460% ± 0.150 | 65.540% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.903% ± 0.043 | 4.399% ± 0.065 | 6.943% ± 0.080 | 93.057% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.361% ± 0.083 | 12.495% ± 0.105 | 87.505% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.551% ± 0.078 | 11.663% ± 0.102 | 16.865% ± 0.118 | 83.135% ± 0.118 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.769% | 29.269% | 17.535% | 10.435% |
| Matchup-flex JIT, going first | 21.357% | 29.191% | 17.833% | 10.370% |
| No discard control, going first | 25.207% | 35.203% | 15.395% | 7.194% |
| Strict JIT, going second | 48.312% | 15.954% | 10.369% | 6.733% |
| Matchup-flex JIT, going second | 51.433% | 15.766% | 9.557% | 6.468% |
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

Simulator policy digest: `1386f235199c4f3e40e59b6df23b4d420dad8e10b4cbfd3f782e48e2d2daf8c2`.

Comparison CSV SHA-256: `b0516d9ef524eaaf3db289b84c4083e22e984ecad4e27ba21de9917405f7df79`.