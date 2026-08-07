# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% | 19.501% | +7.324 pp | 40.905% | 48.517% | +7.612 pp | 57.576% | 66.255% | +8.679 pp |
| Strict JIT, going second | 29.843% | 47.304% | +17.461 pp | 54.073% | 63.315% | +9.242 pp | 65.450% | 73.981% | +8.531 pp |
| Matchup-flex JIT, going first | 16.541% | 21.187% | +4.646 pp | 49.238% | 50.239% | +1.001 pp | 64.738% | 68.403% | +3.665 pp |
| Matchup-flex JIT, going second | 37.565% | 51.204% | +13.639 pp | 61.758% | 67.041% | +5.283 pp | 72.097% | 76.606% | +4.509 pp |
| No discard control, going first | 19.958% | 25.216% | +5.258 pp | 56.020% | 60.418% | +4.398 pp | 72.356% | 75.805% | +3.449 pp |
| No discard control, going second | 39.964% | 59.434% | +19.470 pp | 67.123% | 73.939% | +6.816 pp | 78.368% | 82.494% | +4.126 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% ± 0.103 | 40.905% ± 0.155 | 57.576% ± 0.156 | 42.424% ± 0.156 |
| Matchup-flex JIT, going first | 16.541% ± 0.117 | 49.238% ± 0.158 | 64.738% ± 0.151 | 35.262% ± 0.151 |
| No discard control, going first | 19.958% ± 0.126 | 56.020% ± 0.157 | 72.356% ± 0.141 | 27.644% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.393% ± 0.097 | 18.262% ± 0.122 | 81.738% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.502% ± 0.066 | 26.775% ± 0.140 | 40.202% ± 0.155 | 59.798% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.812% ± 0.028 | 4.674% ± 0.067 | 9.572% ± 0.093 | 90.428% ± 0.093 |
| Strict JIT, going second | 29.843% ± 0.145 | 54.073% ± 0.158 | 65.450% ± 0.150 | 34.550% ± 0.150 |
| Matchup-flex JIT, going second | 37.565% ± 0.153 | 61.758% ± 0.154 | 72.097% ± 0.142 | 27.903% ± 0.142 |
| No discard control, going second | 39.964% ± 0.155 | 67.123% ± 0.149 | 78.368% ± 0.130 | 21.632% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.177% ± 0.110 | 28.393% ± 0.143 | 36.916% ± 0.153 | 63.084% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.253% ± 0.122 | 35.714% ± 0.152 | 46.281% ± 0.158 | 53.719% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.793% ± 0.060 | 14.286% ± 0.111 | 19.840% ± 0.126 | 80.160% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.382% ± 0.114 | 21.683% ± 0.130 | 78.317% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.123% ± 0.086 | 19.412% ± 0.125 | 25.305% ± 0.137 | 74.695% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.177% | 28.728% | 16.671% | 10.522% |
| Matchup-flex JIT, going first | 16.541% | 32.697% | 15.500% | 9.574% |
| No discard control, going first | 19.958% | 36.062% | 16.336% | 9.010% |
| Strict JIT, going second | 29.843% | 24.230% | 11.377% | 7.974% |
| Matchup-flex JIT, going second | 37.565% | 24.193% | 10.339% | 6.977% |
| No discard control, going second | 39.964% | 27.159% | 11.245% | 6.605% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.501% ± 0.125 | 48.517% ± 0.158 | 66.255% ± 0.150 | 33.745% ± 0.150 |
| Matchup-flex JIT, going first | 21.187% ± 0.129 | 50.239% ± 0.158 | 68.403% ± 0.147 | 31.597% ± 0.147 |
| No discard control, going first | 25.216% ± 0.137 | 60.418% ± 0.155 | 75.805% ± 0.135 | 24.195% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.673% ± 0.067 | 8.673% ± 0.089 | 14.888% ± 0.113 | 85.112% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.922% ± 0.068 | 18.479% ± 0.123 | 30.633% ± 0.146 | 69.367% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.968% ± 0.031 | 2.394% ± 0.048 | 4.947% ± 0.069 | 95.053% ± 0.069 |
| Strict JIT, going second | 47.304% ± 0.158 | 63.315% ± 0.152 | 73.981% ± 0.139 | 26.019% ± 0.139 |
| Matchup-flex JIT, going second | 51.204% ± 0.158 | 67.041% ± 0.149 | 76.606% ± 0.134 | 23.394% ± 0.134 |
| No discard control, going second | 59.434% ± 0.155 | 73.939% ± 0.139 | 82.494% ± 0.120 | 17.506% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.438% ± 0.104 | 24.643% ± 0.136 | 34.317% ± 0.150 | 65.683% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.893% ± 0.043 | 4.383% ± 0.065 | 6.938% ± 0.080 | 93.062% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.924% ± 0.043 | 6.895% ± 0.080 | 11.923% ± 0.102 | 88.077% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.376% ± 0.077 | 11.196% ± 0.100 | 16.342% ± 0.117 | 83.658% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.501% | 29.016% | 17.738% | 10.558% |
| Matchup-flex JIT, going first | 21.187% | 29.052% | 18.164% | 10.507% |
| No discard control, going first | 25.216% | 35.202% | 15.387% | 7.192% |
| Strict JIT, going second | 47.304% | 16.011% | 10.666% | 6.887% |
| Matchup-flex JIT, going second | 51.204% | 15.837% | 9.565% | 6.430% |
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

Simulator policy digest: `ce8854d31f6964a2faa5cd66ff384f1fe643b66687338e77cf8a0f76654ecab5`.

Comparison CSV SHA-256: `a2ff378e2b83925622ad98cbeb423c3d50a9354460f8c75f2dd5a7d8df7d2da5`.
