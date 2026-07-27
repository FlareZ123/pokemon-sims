# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.968% | 18.689% | +6.721 pp | 38.801% | 44.498% | +5.697 pp | 55.478% | 61.981% | +6.503 pp |
| Strict JIT, going second | 29.368% | 46.171% | +16.803 pp | 52.662% | 61.094% | +8.432 pp | 63.841% | 71.485% | +7.644 pp |
| Matchup-flex JIT, going first | 16.293% | 19.954% | +3.661 pp | 47.585% | 45.550% | -2.035 pp | 63.366% | 63.326% | -0.040 pp |
| Matchup-flex JIT, going second | 37.372% | 49.271% | +11.899 pp | 60.760% | 64.357% | +3.597 pp | 71.024% | 73.969% | +2.945 pp |
| No discard control, going first | 19.989% | 24.665% | +4.676 pp | 55.828% | 57.875% | +2.047 pp | 71.955% | 73.648% | +1.693 pp |
| No discard control, going second | 40.179% | 58.719% | +18.540 pp | 67.096% | 72.884% | +5.788 pp | 78.314% | 81.449% | +3.135 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.968% ± 0.103 | 38.801% ± 0.154 | 55.478% ± 0.157 | 44.522% ± 0.157 |
| Matchup-flex JIT, going first | 16.293% ± 0.117 | 47.585% ± 0.158 | 63.366% ± 0.152 | 36.634% ± 0.152 |
| No discard control, going first | 19.989% ± 0.126 | 55.828% ± 0.157 | 71.955% ± 0.142 | 28.045% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.442% ± 0.065 | 25.962% ± 0.139 | 38.976% ± 0.154 | 61.024% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.368% ± 0.144 | 52.662% ± 0.158 | 63.841% ± 0.152 | 36.159% ± 0.152 |
| Matchup-flex JIT, going second | 37.372% ± 0.153 | 60.760% ± 0.154 | 71.024% ± 0.143 | 28.976% ± 0.143 |
| No discard control, going second | 40.179% ± 0.155 | 67.096% ± 0.149 | 78.314% ± 0.130 | 21.686% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.081% ± 0.110 | 27.971% ± 0.142 | 35.630% ± 0.151 | 64.370% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.116% ± 0.122 | 34.665% ± 0.150 | 44.754% ± 0.157 | 55.246% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.282% ± 0.114 | 21.547% ± 0.130 | 78.453% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.428% ± 0.125 | 25.336% ± 0.138 | 74.664% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.968% | 26.833% | 16.677% | 10.496% |
| Matchup-flex JIT, going first | 16.293% | 31.292% | 15.781% | 9.610% |
| No discard control, going first | 19.989% | 35.839% | 16.127% | 8.954% |
| Strict JIT, going second | 29.368% | 23.294% | 11.179% | 7.878% |
| Matchup-flex JIT, going second | 37.372% | 23.388% | 10.264% | 6.991% |
| No discard control, going second | 40.179% | 26.917% | 11.218% | 6.565% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.689% ± 0.123 | 44.498% ± 0.157 | 61.981% ± 0.154 | 38.019% ± 0.154 |
| Matchup-flex JIT, going first | 19.954% ± 0.126 | 45.550% ± 0.157 | 63.326% ± 0.152 | 36.674% ± 0.152 |
| No discard control, going first | 24.665% ± 0.136 | 57.875% ± 0.156 | 73.648% ± 0.139 | 26.352% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.688% ± 0.067 | 16.614% ± 0.118 | 27.633% ± 0.141 | 72.367% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.171% ± 0.158 | 61.094% ± 0.154 | 71.485% ± 0.143 | 28.515% ± 0.143 |
| Matchup-flex JIT, going second | 49.271% ± 0.158 | 64.357% ± 0.151 | 73.969% ± 0.139 | 26.031% ± 0.139 |
| No discard control, going second | 58.719% ± 0.156 | 72.884% ± 0.141 | 81.449% ± 0.123 | 18.551% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.612% ± 0.079 | 11.839% ± 0.102 | 17.549% ± 0.120 | 82.451% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.767% ± 0.102 | 22.415% ± 0.132 | 31.664% ± 0.147 | 68.336% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 5.803% ± 0.074 | 9.958% ± 0.095 | 90.042% ± 0.095 |
| Strict JIT, Supporter lock, second | 6.022% ± 0.075 | 9.967% ± 0.095 | 14.277% ± 0.111 | 85.723% ± 0.111 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.689% | 25.809% | 17.483% | 10.526% |
| Matchup-flex JIT, going first | 19.954% | 25.596% | 17.776% | 10.449% |
| No discard control, going first | 24.665% | 33.210% | 15.773% | 7.970% |
| Strict JIT, going second | 46.171% | 14.923% | 10.391% | 6.509% |
| Matchup-flex JIT, going second | 49.271% | 15.086% | 9.612% | 6.143% |
| No discard control, going second | 58.719% | 14.165% | 8.565% | 4.871% |

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
| Secret Box use | 62.970% |
| Exploding Energy use | 78.174% |
| Steven use | 36.219% |
| Star Alchemy use | 48.368% |
| Secret Box attempts | 1.542 per game |
| Cost blocks | 0.045 per game |
| Missing route axis | 0.865 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.273 per game |
| Steven banks | 0.306 per game |
| Gladion banks | 0.040 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.404 |
| Pineco/Forretress line | 0.475 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.028 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.166 |
| Known Prize zone | 0.520 |
| Discard zone | 0.284 |
| Stranded hand zone | 0.212 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `eecc5fbbfbd052aff9b9beb66cfc28d55ee3ec6877a2f709453ae06dfc968636`.

Comparison CSV SHA-256: `5618a15756591d3e8675e0ab2a8b24c150fff2d28518f9fa7baed34e78fb4222`.
