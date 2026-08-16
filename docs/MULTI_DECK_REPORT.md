# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% | 19.554% | +7.396 pp | 41.807% | 48.970% | +7.163 pp | 59.222% | 66.762% | +7.540 pp |
| Strict JIT, going second | 29.667% | 48.516% | +18.849 pp | 55.561% | 64.495% | +8.934 pp | 67.523% | 74.919% | +7.396 pp |
| Matchup-flex JIT, going first | 17.172% | 21.321% | +4.149 pp | 50.772% | 51.203% | +0.431 pp | 68.179% | 69.208% | +1.029 pp |
| Matchup-flex JIT, going second | 37.321% | 51.672% | +14.351 pp | 64.185% | 67.753% | +3.568 pp | 75.793% | 77.242% | +1.449 pp |
| No discard control, going first | 19.740% | 25.271% | +5.531 pp | 56.829% | 60.276% | +3.447 pp | 73.585% | 75.715% | +2.130 pp |
| No discard control, going second | 40.232% | 59.294% | +19.062 pp | 68.436% | 73.788% | +5.352 pp | 79.981% | 82.363% | +2.382 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% ± 0.103 | 41.807% ± 0.156 | 59.222% ± 0.155 | 40.778% ± 0.155 |
| Matchup-flex JIT, going first | 17.172% ± 0.119 | 50.772% ± 0.158 | 68.179% ± 0.147 | 31.821% ± 0.147 |
| No discard control, going first | 19.740% ± 0.126 | 56.829% ± 0.157 | 73.585% ± 0.139 | 26.415% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.583% ± 0.066 | 10.484% ± 0.097 | 18.587% ± 0.123 | 81.413% ± 0.123 |
| Strict JIT, Rule Box Ability lock, first | 4.539% ± 0.066 | 27.748% ± 0.142 | 42.696% ± 0.156 | 57.304% ± 0.156 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% ± 0.028 | 4.667% ± 0.067 | 9.645% ± 0.093 | 90.355% ± 0.093 |
| Strict JIT, going second | 29.667% ± 0.144 | 55.561% ± 0.157 | 67.523% ± 0.148 | 32.477% ± 0.148 |
| Matchup-flex JIT, going second | 37.321% ± 0.153 | 64.185% ± 0.152 | 75.793% ± 0.135 | 24.207% ± 0.135 |
| No discard control, going second | 40.232% ± 0.155 | 68.436% ± 0.147 | 79.981% ± 0.127 | 20.019% ± 0.127 |
| Strict JIT, turn-two Item lock, second | 14.203% ± 0.110 | 28.493% ± 0.143 | 37.272% ± 0.153 | 62.728% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.561% ± 0.123 | 37.171% ± 0.153 | 49.433% ± 0.158 | 50.567% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.924% ± 0.061 | 14.705% ± 0.112 | 20.389% ± 0.127 | 79.611% ± 0.127 |
| Strict JIT, Supporter lock, first | 0.005% ± 0.002 | 16.426% ± 0.117 | 23.376% ± 0.134 | 76.624% ± 0.134 |
| Strict JIT, Supporter lock, second | 7.850% ± 0.085 | 20.357% ± 0.127 | 26.735% ± 0.140 | 73.265% ± 0.140 |
| Garbodor + Boost Shake Ability lock, first | 5.604% ± 0.073 | 25.133% ± 0.137 | 37.799% ± 0.153 | 62.201% ± 0.153 |
| Garbodor + Boost Shake Ability lock, second | 15.389% ± 0.114 | 30.670% ± 0.146 | 42.053% ± 0.156 | 57.947% ± 0.156 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.158% | 29.649% | 17.415% | 10.940% |
| Matchup-flex JIT, going first | 17.172% | 33.600% | 17.407% | 10.033% |
| No discard control, going first | 19.740% | 37.089% | 16.756% | 9.045% |
| Strict JIT, going second | 29.667% | 25.894% | 11.962% | 7.962% |
| Matchup-flex JIT, going second | 37.321% | 26.864% | 11.608% | 6.927% |
| No discard control, going second | 40.232% | 28.204% | 11.545% | 6.384% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.554% ± 0.125 | 48.970% ± 0.158 | 66.762% ± 0.149 | 33.238% ± 0.149 |
| Matchup-flex JIT, going first | 21.321% ± 0.130 | 51.203% ± 0.158 | 69.208% ± 0.146 | 30.792% ± 0.146 |
| No discard control, going first | 25.271% ± 0.137 | 60.276% ± 0.155 | 75.715% ± 0.136 | 24.285% ± 0.136 |
| Strict JIT, turn-two Item lock, first | 4.655% ± 0.067 | 8.643% ± 0.089 | 14.924% ± 0.113 | 85.076% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 5.278% ± 0.071 | 19.773% ± 0.126 | 32.337% ± 0.148 | 67.663% ± 0.148 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 1.177% ± 0.034 | 2.646% ± 0.051 | 5.329% ± 0.071 | 94.671% ± 0.071 |
| Strict JIT, going second | 48.516% ± 0.158 | 64.495% ± 0.151 | 74.919% ± 0.137 | 25.081% ± 0.137 |
| Matchup-flex JIT, going second | 51.672% ± 0.158 | 67.753% ± 0.148 | 77.242% ± 0.133 | 22.758% ± 0.133 |
| No discard control, going second | 59.294% ± 0.155 | 73.788% ± 0.139 | 82.363% ± 0.121 | 17.637% ± 0.121 |
| Strict JIT, turn-two Item lock, second | 6.797% ± 0.080 | 12.785% ± 0.106 | 19.062% ± 0.124 | 80.938% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 13.270% ± 0.107 | 25.828% ± 0.138 | 35.887% ± 0.152 | 64.113% ± 0.152 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.975% ± 0.044 | 4.637% ± 0.066 | 7.306% ± 0.082 | 92.694% ± 0.082 |
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

Simulator policy digest: `f6b33716d276ebc706fc82ad49db265d1558c03d208a0082c0536481bee90318`.

Comparison CSV SHA-256: `a3e4e273634f06a45e11e1a03f883f3100332d07e12bcbb9715b2bfc1a8c72ad`.