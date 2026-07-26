# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% | 18.707% | +6.714 pp | 38.720% | 44.488% | +5.768 pp | 55.422% | 61.976% | +6.554 pp |
| Strict JIT, going second | 29.399% | 46.166% | +16.767 pp | 52.700% | 61.077% | +8.377 pp | 63.901% | 71.474% | +7.573 pp |
| Matchup-flex JIT, going first | 16.252% | 19.998% | +3.746 pp | 47.367% | 45.587% | -1.780 pp | 63.306% | 63.328% | +0.022 pp |
| Matchup-flex JIT, going second | 37.311% | 49.254% | +11.943 pp | 60.701% | 64.355% | +3.654 pp | 70.987% | 73.964% | +2.977 pp |
| No discard control, going first | 19.989% | 24.665% | +4.676 pp | 55.828% | 57.875% | +2.047 pp | 71.955% | 73.648% | +1.693 pp |
| No discard control, going second | 40.179% | 58.720% | +18.541 pp | 67.096% | 72.881% | +5.785 pp | 78.314% | 81.446% | +3.132 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% ± 0.103 | 38.720% ± 0.154 | 55.422% ± 0.157 | 44.578% ± 0.157 |
| Matchup-flex JIT, going first | 16.252% ± 0.117 | 47.367% ± 0.158 | 63.306% ± 0.152 | 36.694% ± 0.152 |
| No discard control, going first | 19.989% ± 0.126 | 55.828% ± 0.157 | 71.955% ± 0.142 | 28.045% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.413% ± 0.065 | 25.917% ± 0.139 | 38.946% ± 0.154 | 61.054% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.399% ± 0.144 | 52.700% ± 0.158 | 63.901% ± 0.152 | 36.099% ± 0.152 |
| Matchup-flex JIT, going second | 37.311% ± 0.153 | 60.701% ± 0.154 | 70.987% ± 0.144 | 29.013% ± 0.144 |
| No discard control, going second | 40.179% ± 0.155 | 67.096% ± 0.149 | 78.314% ± 0.130 | 21.686% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.081% ± 0.110 | 27.971% ± 0.142 | 35.630% ± 0.151 | 64.370% ± 0.151 |
| Strict JIT, full Item lock, second | 10.531% ± 0.097 | 22.928% ± 0.133 | 30.088% ± 0.145 | 69.912% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.095% ± 0.122 | 34.662% ± 0.150 | 44.792% ± 0.157 | 55.208% ± 0.157 |
| Strict JIT, combined lock, second | 2.368% ± 0.048 | 11.389% ± 0.100 | 15.503% ± 0.114 | 84.497% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.004% ± 0.002 | 15.282% ± 0.114 | 21.547% ± 0.130 | 78.453% ± 0.130 |
| Strict JIT, Supporter lock, second | 8.099% ± 0.086 | 19.428% ± 0.125 | 25.336% ± 0.138 | 74.664% ± 0.138 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% | 26.727% | 16.702% | 10.538% |
| Matchup-flex JIT, going first | 16.252% | 31.115% | 15.939% | 9.662% |
| No discard control, going first | 19.989% | 35.839% | 16.127% | 8.954% |
| Strict JIT, going second | 29.399% | 23.301% | 11.201% | 7.866% |
| Matchup-flex JIT, going second | 37.311% | 23.390% | 10.286% | 7.002% |
| No discard control, going second | 40.179% | 26.917% | 11.218% | 6.565% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.707% ± 0.123 | 44.488% ± 0.157 | 61.976% ± 0.154 | 38.024% ± 0.154 |
| Matchup-flex JIT, going first | 19.998% ± 0.126 | 45.587% ± 0.157 | 63.328% ± 0.152 | 36.672% ± 0.152 |
| No discard control, going first | 24.665% ± 0.136 | 57.875% ± 0.156 | 73.648% ± 0.139 | 26.352% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.658% ± 0.067 | 16.493% ± 0.117 | 27.582% ± 0.141 | 72.418% ± 0.141 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.166% ± 0.158 | 61.077% ± 0.154 | 71.474% ± 0.143 | 28.526% ± 0.143 |
| Matchup-flex JIT, going second | 49.254% ± 0.158 | 64.355% ± 0.151 | 73.964% ± 0.139 | 26.036% ± 0.139 |
| No discard control, going second | 58.720% ± 0.156 | 72.881% ± 0.141 | 81.446% ± 0.123 | 18.554% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.612% ± 0.079 | 11.839% ± 0.102 | 17.549% ± 0.120 | 82.451% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.743% ± 0.102 | 22.357% ± 0.132 | 31.611% ± 0.147 | 68.389% ± 0.147 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.865% ± 0.043 | 5.803% ± 0.074 | 9.958% ± 0.095 | 90.042% ± 0.095 |
| Strict JIT, Supporter lock, second | 6.022% ± 0.075 | 9.967% ± 0.095 | 14.277% ± 0.111 | 85.723% ± 0.111 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.707% | 25.781% | 17.488% | 10.540% |
| Matchup-flex JIT, going first | 19.998% | 25.589% | 17.741% | 10.451% |
| No discard control, going first | 24.665% | 33.210% | 15.773% | 7.970% |
| Strict JIT, going second | 46.166% | 14.911% | 10.397% | 6.514% |
| Matchup-flex JIT, going second | 49.254% | 15.101% | 9.609% | 6.143% |
| No discard control, going second | 58.720% | 14.161% | 8.565% | 4.873% |

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
| Secret Box use | 62.980% |
| Exploding Energy use | 78.176% |
| Steven use | 36.220% |
| Star Alchemy use | 48.375% |
| Secret Box attempts | 1.542 per game |
| Cost blocks | 0.045 per game |
| Missing route axis | 0.865 per game |
| Bench blocks | 0.002 per game |
| Arven banks | 0.274 per game |
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

Simulator policy digest: `e6c093464dcd475876cda4c23ba506394cfcf140dda1c33bf966e4c2fc0072d2`.

Comparison CSV SHA-256: `e3880459d768eb40979d6e86958ac2b9af7eda346a88c8796bcc8ae3476081de`.
