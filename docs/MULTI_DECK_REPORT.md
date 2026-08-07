# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `28`. Total simulated games: `2,800,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. Use the turn-two Item-lock rows. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 12.212% | 19.541% | +7.329 pp | 40.871% | 48.733% | +7.862 pp | 57.677% | 66.396% | +8.719 pp |
| Strict JIT, going second | 29.742% | 47.661% | +17.919 pp | 54.024% | 63.577% | +9.553 pp | 65.429% | 74.128% | +8.699 pp |
| Matchup-flex JIT, going first | 16.594% | 21.113% | +4.519 pp | 49.484% | 50.191% | +0.707 pp | 65.212% | 68.455% | +3.243 pp |
| Matchup-flex JIT, going second | 37.847% | 51.224% | +13.377 pp | 62.366% | 66.999% | +4.633 pp | 72.872% | 76.586% | +3.714 pp |
| No discard control, going first | 20.034% | 25.524% | +5.490 pp | 56.984% | 60.602% | +3.618 pp | 73.423% | 76.187% | +2.764 pp |
| No discard control, going second | 40.347% | 59.742% | +19.395 pp | 68.342% | 74.290% | +5.948 pp | 79.382% | 82.772% | +3.390 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.212% ± 0.104 | 40.871% ± 0.155 | 57.677% ± 0.156 | 42.323% ± 0.156 |
| Matchup-flex JIT, going first | 16.594% ± 0.118 | 49.484% ± 0.158 | 65.212% ± 0.151 | 34.788% ± 0.151 |
| No discard control, going first | 20.034% ± 0.127 | 56.984% ± 0.157 | 73.423% ± 0.140 | 26.577% ± 0.140 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.393% ± 0.097 | 18.262% ± 0.122 | 81.738% ± 0.122 |
| Strict JIT, Rule Box Ability lock, first | 4.373% ± 0.065 | 26.666% ± 0.140 | 40.285% ± 0.155 | 59.715% ± 0.155 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.812% ± 0.028 | 4.674% ± 0.067 | 9.572% ± 0.093 | 90.428% ± 0.093 |
| Strict JIT, going second | 29.742% ± 0.145 | 54.024% ± 0.158 | 65.429% ± 0.150 | 34.571% ± 0.150 |
| Matchup-flex JIT, going second | 37.847% ± 0.153 | 62.366% ± 0.153 | 72.872% ± 0.141 | 27.128% ± 0.141 |
| No discard control, going second | 40.347% ± 0.155 | 68.342% ± 0.147 | 79.382% ± 0.128 | 20.618% ± 0.128 |
| Strict JIT, turn-two Item lock, second | 14.177% ± 0.110 | 28.393% ± 0.143 | 36.916% ± 0.153 | 63.084% ± 0.153 |
| Strict JIT, Rule Box Ability lock, second | 18.295% ± 0.122 | 35.681% ± 0.151 | 46.452% ± 0.158 | 53.548% ± 0.158 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.793% ± 0.060 | 14.286% ± 0.111 | 19.840% ± 0.126 | 80.160% ± 0.126 |
| Strict JIT, Supporter lock, first | 0.003% ± 0.002 | 15.422% ± 0.114 | 21.773% ± 0.131 | 78.227% ± 0.131 |
| Strict JIT, Supporter lock, second | 8.120% ± 0.086 | 19.416% ± 0.125 | 25.348% ± 0.138 | 74.652% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 12.212% | 28.659% | 16.806% | 10.586% |
| Matchup-flex JIT, going first | 16.594% | 32.890% | 15.728% | 9.574% |
| No discard control, going first | 20.034% | 36.950% | 16.439% | 8.882% |
| Strict JIT, going second | 29.742% | 24.282% | 11.405% | 8.111% |
| Matchup-flex JIT, going second | 37.847% | 24.519% | 10.506% | 6.788% |
| No discard control, going second | 40.347% | 27.995% | 11.040% | 6.354% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.541% ± 0.125 | 48.733% ± 0.158 | 66.396% ± 0.149 | 33.604% ± 0.149 |
| Matchup-flex JIT, going first | 21.113% ± 0.129 | 50.191% ± 0.158 | 68.455% ± 0.147 | 31.545% ± 0.147 |
| No discard control, going first | 25.524% ± 0.138 | 60.602% ± 0.155 | 76.187% ± 0.135 | 23.813% ± 0.135 |
| Strict JIT, turn-two Item lock, first | 4.673% ± 0.067 | 8.673% ± 0.089 | 14.888% ± 0.113 | 85.112% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.956% ± 0.069 | 18.543% ± 0.123 | 30.617% ± 0.146 | 69.383% ± 0.146 |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.968% ± 0.031 | 2.394% ± 0.048 | 4.947% ± 0.069 | 95.053% ± 0.069 |
| Strict JIT, going second | 47.661% ± 0.158 | 63.577% ± 0.152 | 74.128% ± 0.138 | 25.872% ± 0.138 |
| Matchup-flex JIT, going second | 51.224% ± 0.158 | 66.999% ± 0.149 | 76.586% ± 0.134 | 23.414% ± 0.134 |
| No discard control, going second | 59.742% ± 0.155 | 74.290% ± 0.138 | 82.772% ± 0.119 | 17.228% ± 0.119 |
| Strict JIT, turn-two Item lock, second | 6.890% ± 0.080 | 12.786% ± 0.106 | 19.132% ± 0.124 | 80.868% ± 0.124 |
| Strict JIT, Rule Box Ability lock, second | 12.513% ± 0.105 | 24.746% ± 0.136 | 34.428% ± 0.150 | 65.572% ± 0.150 |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 1.893% ± 0.043 | 4.383% ± 0.065 | 6.938% ± 0.080 | 93.062% ± 0.080 |
| Strict JIT, Supporter lock, first | 1.924% ± 0.043 | 6.895% ± 0.080 | 11.923% ± 0.102 | 88.077% ± 0.102 |
| Strict JIT, Supporter lock, second | 6.376% ± 0.077 | 11.196% ± 0.100 | 16.342% ± 0.117 | 83.658% ± 0.117 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 19.541% | 29.192% | 17.663% | 10.529% |
| Matchup-flex JIT, going first | 21.113% | 29.078% | 18.264% | 10.464% |
| No discard control, going first | 25.524% | 35.078% | 15.585% | 7.217% |
| Strict JIT, going second | 47.661% | 15.916% | 10.551% | 6.848% |
| Matchup-flex JIT, going second | 51.224% | 15.775% | 9.587% | 6.436% |
| No discard control, going second | 59.742% | 14.548% | 8.482% | 4.697% |

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
| Secret Box use | 64.276% |
| Exploding Energy use | 78.469% |
| Steven use | 35.820% |
| Star Alchemy use | 48.097% |
| Secret Box attempts | 1.449 per game |
| Cost blocks | 0.049 per game |
| Missing route axis | 0.756 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.273 per game |
| Steven banks | 0.302 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.388 |
| Pineco/Forretress line | 0.417 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.003 |
| Fire | 0.024 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.154 |
| Known Prize zone | 0.460 |
| Discard zone | 0.232 |
| Stranded hand zone | 0.185 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `27cd6254bfd66d93edff37c17a20f5def8cc604746bae6e63b8de8f6b244fb4d`.

Comparison CSV SHA-256: `25845e65120e383679ede62b9b3bb8bc5eef5bb6f600af13fb8ac22a3750010c`.
