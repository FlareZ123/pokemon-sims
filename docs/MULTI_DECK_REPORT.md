# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.080% | 19.660% | +7.580 pp | 41.196% | 49.065% | +7.869 pp | 58.185% | 66.560% | +8.375 pp |
| Strict JIT, going second | 29.598% | 48.155% | +18.557 pp | 54.380% | 63.891% | +9.511 pp | 66.120% | 74.377% | +8.257 pp |
| Matchup-flex JIT, going first | 16.943% | 21.268% | +4.325 pp | 49.967% | 50.664% | +0.697 pp | 65.891% | 68.625% | +2.734 pp |
| Matchup-flex JIT, going second | 37.471% | 51.506% | +14.035 pp | 62.401% | 67.278% | +4.877 pp | 72.985% | 76.947% | +3.962 pp |
| No discard control, going first | 19.971% | 25.205% | +5.234 pp | 56.026% | 60.428% | +4.402 pp | 72.341% | 75.796% | +3.455 pp |
| No discard control, going second | 39.971% | 59.447% | +19.476 pp | 67.161% | 73.947% | +6.786 pp | 78.402% | 82.479% | +4.077 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.080% ± 0.103 | 41.196% ± 0.156 | 58.185% ± 0.156 | 41.815% ± 0.156 |
| Matchup-flex JIT, going first | 16.943% ± 0.119 | 49.967% ± 0.158 | 65.891% ± 0.150 | 34.109% ± 0.150 |
| No discard control, going first | 19.971% ± 0.126 | 56.026% ± 0.157 | 72.341% ± 0.141 | 27.659% ± 0.141 |
| Strict JIT, turn-two Item lock, first | 4.617% ± 0.066 | 10.410% ± 0.097 | 18.285% ± 0.122 | 81.715% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.550% ± 0.066 | 26.996% ± 0.140 | 40.657% ± 0.155 | 59.343% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.801% ± 0.028 | 4.633% ± 0.066 | 9.537% ± 0.093 | 90.463% ± 0.093 |
| Strict JIT, going second | 29.598% ± 0.144 | 54.380% ± 0.158 | 66.120% ± 0.150 | 33.880% ± 0.150 |
| Matchup-flex JIT, going second | 37.471% ± 0.153 | 62.401% ± 0.153 | 72.985% ± 0.140 | 27.015% ± 0.140 |
| No discard control, going second | 39.971% ± 0.155 | 67.161% ± 0.149 | 78.402% ± 0.130 | 21.598% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.203% ± 0.110 | 28.479% ± 0.143 | 37.045% ± 0.153 | 62.955% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.472% ± 0.123 | 35.949% ± 0.152 | 46.933% ± 0.158 | 53.067% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.767% ± 0.060 | 14.289% ± 0.111 | 19.883% ± 0.126 | 80.117% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 14.719% ± 0.112 | 21.186% ± 0.129 | 78.814% ± 0.129 |
| Strict JIT, Supporter lock, second | 8.046% ± 0.086 | 19.303% ± 0.125 | 25.257% ± 0.137 | 74.743% ± 0.137 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.080% | 29.116% | 16.989% | 10.546% |
| Matchup-flex JIT, going first | 16.943% | 33.024% | 15.924% | 9.559% |
| No discard control, going first | 19.971% | 36.055% | 16.315% | 9.022% |
| Strict JIT, going second | 29.598% | 24.782% | 11.740% | 7.933% |
| Matchup-flex JIT, going second | 37.471% | 24.930% | 10.584% | 7.126% |
| No discard control, going second | 39.971% | 27.190% | 11.241% | 6.628% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.660% ± 0.126 | 49.065% ± 0.158 | 66.560% ± 0.149 | 33.440% ± 0.149 |
| Matchup-flex JIT, going first | 21.268% ± 0.129 | 50.664% ± 0.158 | 68.625% ± 0.147 | 31.375% ± 0.147 |
| No discard control, going first | 25.205% ± 0.137 | 60.428% ± 0.155 | 75.796% ± 0.135 | 24.204% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.671% ± 0.067 | 8.666% ± 0.089 | 14.920% ± 0.113 | 85.080% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.304% ± 0.071 | 18.666% ± 0.123 | 30.797% ± 0.146 | 69.203% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.250% ± 0.035 | 2.646% ± 0.051 | 5.092% ± 0.070 | 94.908% ± 0.070 |
| Strict JIT, going second | 48.155% ± 0.158 | 63.891% ± 0.152 | 74.377% ± 0.138 | 25.623% ± 0.138 |
| Matchup-flex JIT, going second | 51.506% ± 0.158 | 67.278% ± 0.148 | 76.947% ± 0.133 | 23.053% ± 0.133 |
| No discard control, going second | 59.447% ± 0.155 | 73.947% ± 0.139 | 82.479% ± 0.120 | 17.521% ± 0.120 |
| Strict JIT, turn-two Item lock, second | 6.852% ± 0.080 | 12.792% ± 0.106 | 19.137% ± 0.124 | 80.863% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.577% ± 0.105 | 24.687% ± 0.136 | 34.465% ± 0.150 | 65.535% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.900% ± 0.043 | 4.372% ± 0.065 | 6.902% ± 0.080 | 93.098% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.897% ± 0.043 | 7.361% ± 0.083 | 12.495% ± 0.105 | 87.505% ± 0.105 |
| Strict JIT, Supporter lock, second | 6.551% ± 0.078 | 11.663% ± 0.102 | 16.865% ± 0.118 | 83.135% ± 0.118 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.660% | 29.405% | 17.495% | 10.379% |
| Matchup-flex JIT, going first | 21.268% | 29.396% | 17.961% | 10.201% |
| No discard control, going first | 25.205% | 35.223% | 15.368% | 7.179% |
| Strict JIT, going second | 48.155% | 15.736% | 10.486% | 6.732% |
| Matchup-flex JIT, going second | 51.506% | 15.772% | 9.669% | 6.269% |
| No discard control, going second | 59.447% | 14.500% | 8.532% | 4.646% |

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
| Secret Box use | 64.317% |
| Exploding Energy use | 78.599% |
| Steven use | 35.800% |
| Star Alchemy use | 48.147% |
| Secret Box attempts | 1.465 per game |
| Cost blocks | 0.047 per game |
| Missing route axis | 0.774 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.400 |
| Pineco/Forretress line | 0.425 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.025 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.472 |
| Discard zone | 0.238 |
| Stranded hand zone | 0.190 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `31f9fa3926bb0627d6a8cc67976d3cdeddf5d4b40559570a9c5a4bebe5a06498`.

Comparison CSV SHA-256: `387e4cb2648cb5ec4bc8fb01c75eb0e50b233c036618d72eda166000e7539438`.
