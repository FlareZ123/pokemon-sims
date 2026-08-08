# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% | 19.557% | +7.332 pp | 41.192% | 48.700% | +7.508 pp | 58.066% | 66.396% | +8.330 pp |
| Strict JIT, going second | 29.659% | 47.891% | +18.232 pp | 54.239% | 63.736% | +9.497 pp | 65.761% | 74.315% | +8.554 pp |
| Matchup-flex JIT, going first | 16.399% | 21.147% | +4.748 pp | 49.418% | 50.311% | +0.893 pp | 65.296% | 68.516% | +3.220 pp |
| Matchup-flex JIT, going second | 37.065% | 51.262% | +14.197 pp | 62.145% | 67.163% | +5.018 pp | 72.776% | 76.796% | +4.020 pp |
| No discard control, going first | 19.958% | 25.216% | +5.258 pp | 56.020% | 60.418% | +4.398 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.122% | 73.939% | +6.817 pp | 78.369% | 82.494% | +4.125 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% ± 0.104 | 41.192% ± 0.156 | 58.066% ± 0.156 | 41.934% ± 0.156 |
| Matchup-flex JIT, going first | 16.399% ± 0.117 | 49.418% ± 0.158 | 65.296% ± 0.151 | 34.704% ± 0.151 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.588% ± 0.066 | 10.403% ± 0.097 | 18.291% ± 0.122 | 81.709% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.438% ± 0.065 | 26.798% ± 0.140 | 40.304% ± 0.155 | 59.696% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.814% ± 0.028 | 4.665% ± 0.067 | 9.585% ± 0.093 | 90.415% ± 0.093 |
| Strict JIT, going second | 29.659% ± 0.144 | 54.239% ± 0.158 | 65.761% ± 0.150 | 34.239% ± 0.150 |
| Matchup-flex JIT, going second | 37.065% ± 0.153 | 62.145% ± 0.153 | 72.776% ± 0.141 | 27.224% ± 0.141 |
| No discard control, going second | 39.964% ± 0.155 | 67.122% ± 0.149 | 78.369% ± 0.130 | 21.631% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.250% ± 0.111 | 28.427% ± 0.143 | 36.938% ± 0.153 | 63.062% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.372% ± 0.122 | 35.749% ± 0.152 | 46.585% ± 0.158 | 53.415% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.797% ± 0.060 | 14.322% ± 0.111 | 19.877% ± 0.126 | 80.123% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 14.755% ± 0.112 | 21.096% ± 0.129 | 78.904% ± 0.129 |
| Strict JIT, Supporter lock, second | 8.114% ± 0.086 | 19.381% ± 0.125 | 25.192% ± 0.137 | 74.808% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.225% | 28.967% | 16.874% | 10.517% |
| Matchup-flex JIT, going first | 16.399% | 33.019% | 15.878% | 9.630% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.659% | 24.580% | 11.522% | 8.050% |
| Matchup-flex JIT, going second | 37.065% | 25.080% | 10.631% | 7.075% |
| No discard control, going second | 39.964% | 27.158% | 11.247% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.557% ± 0.125 | 48.700% ± 0.158 | 66.396% ± 0.149 | 33.604% ± 0.149 |
| Matchup-flex JIT, going first | 21.147% ± 0.129 | 50.311% ± 0.158 | 68.516% ± 0.147 | 31.484% ± 0.147 |
| No discard control, going first | 25.216% ± 0.137 | 60.418% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.656% ± 0.067 | 8.665% ± 0.089 | 14.898% ± 0.113 | 85.102% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.912% ± 0.068 | 18.579% ± 0.123 | 30.655% ± 0.146 | 69.345% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.973% ± 0.031 | 2.399% ± 0.048 | 4.953% ± 0.069 | 95.047% ± 0.069 |
| Strict JIT, going second | 47.891% ± 0.158 | 63.736% ± 0.152 | 74.315% ± 0.138 | 25.685% ± 0.138 |
| Matchup-flex JIT, going second | 51.262% ± 0.158 | 67.163% ± 0.149 | 76.796% ± 0.133 | 23.204% ± 0.133 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.866% ± 0.080 | 12.754% ± 0.105 | 19.120% ± 0.124 | 80.880% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.543% ± 0.105 | 24.772% ± 0.137 | 34.448% ± 0.150 | 65.552% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.903% ± 0.043 | 4.399% ± 0.065 | 6.943% ± 0.080 | 93.057% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.358% ± 0.083 | 12.485% ± 0.105 | 87.515% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.546% ± 0.078 | 11.657% ± 0.101 | 16.859% ± 0.118 | 83.141% ± 0.118 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.557% | 29.143% | 17.696% | 10.509% |
| Matchup-flex JIT, going first | 21.147% | 29.164% | 18.205% | 10.478% |
| No discard control, going first | 25.216% | 35.202% | 15.387% | 7.192% |
| Strict JIT, going second | 47.891% | 15.845% | 10.579% | 6.734% |
| Matchup-flex JIT, going second | 51.262% | 15.901% | 9.633% | 6.411% |
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

Simulator policy digest: `468fd2302ea0f9b5a68046431b12d35808a9f5b6aec5f26af629beb35784d81a`.

Comparison CSV SHA-256: `a738606f35df733d2d2826728d15fc48b92ce7e921067e1d1bb8c45566a502d1`.
