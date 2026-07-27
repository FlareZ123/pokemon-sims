# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% | 18.482% | +6.489 pp | 38.720% | 44.846% | +6.126 pp | 55.422% | 62.683% | +7.261 pp |
| Strict JIT, going second | 29.399% | 46.468% | +17.069 pp | 52.700% | 61.809% | +9.109 pp | 63.901% | 72.549% | +8.648 pp |
| Matchup-flex JIT, going first | 16.252% | 19.939% | +3.687 pp | 47.367% | 46.317% | -1.050 pp | 63.306% | 64.394% | +1.088 pp |
| Matchup-flex JIT, going second | 37.311% | 49.155% | +11.844 pp | 60.701% | 64.345% | +3.644 pp | 70.987% | 74.369% | +3.382 pp |
| No discard control, going first | 20.001% | 24.848% | +4.847 pp | 55.867% | 59.130% | +3.263 pp | 71.936% | 74.572% | +2.636 pp |
| No discard control, going second | 40.082% | 58.697% | +18.615 pp | 67.019% | 73.223% | +6.204 pp | 78.185% | 81.759% | +3.574 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.993% ± 0.103 | 38.720% ± 0.154 | 55.422% ± 0.157 | 44.578% ± 0.157 |
| Matchup-flex JIT, going first | 16.252% ± 0.117 | 47.367% ± 0.158 | 63.306% ± 0.152 | 36.694% ± 0.152 |
| No discard control, going first | 20.001% ± 0.126 | 55.867% ± 0.157 | 71.936% ± 0.142 | 28.064% ± 0.142 |
| Strict JIT, turn-two Item lock, first | 4.601% ± 0.066 | 10.191% ± 0.096 | 17.704% ± 0.121 | 82.296% ± 0.121 |
| Strict JIT, full Item lock, first | 2.825% ± 0.052 | 7.752% ± 0.085 | 15.070% ± 0.113 | 84.930% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.413% ± 0.065 | 25.917% ± 0.139 | 38.946% ± 0.154 | 61.054% ± 0.154 |
| Strict JIT, combined lock, first | 0.291% ± 0.017 | 3.268% ± 0.056 | 7.254% ± 0.082 | 92.746% ± 0.082 |
| Strict JIT, going second | 29.399% ± 0.144 | 52.700% ± 0.158 | 63.901% ± 0.152 | 36.099% ± 0.152 |
| Matchup-flex JIT, going second | 37.311% ± 0.153 | 60.701% ± 0.154 | 70.987% ± 0.144 | 29.013% ± 0.144 |
| No discard control, going second | 40.082% ± 0.155 | 67.019% ± 0.149 | 78.185% ± 0.131 | 21.815% ± 0.131 |
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
| No discard control, going first | 20.001% | 35.866% | 16.069% | 8.960% |
| Strict JIT, going second | 29.399% | 23.301% | 11.201% | 7.866% |
| Matchup-flex JIT, going second | 37.311% | 23.390% | 10.286% | 7.002% |
| No discard control, going second | 40.082% | 26.937% | 11.166% | 6.597% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.482% ± 0.123 | 44.846% ± 0.157 | 62.683% ± 0.153 | 37.317% ± 0.153 |
| Matchup-flex JIT, going first | 19.939% ± 0.126 | 46.317% ± 0.158 | 64.394% ± 0.151 | 35.606% ± 0.151 |
| No discard control, going first | 24.848% ± 0.137 | 59.130% ± 0.155 | 74.572% ± 0.138 | 25.428% ± 0.138 |
| Strict JIT, turn-two Item lock, first | 4.506% ± 0.066 | 7.697% ± 0.084 | 13.268% ± 0.107 | 86.732% ± 0.107 |
| Strict JIT, full Item lock, first | 2.777% ± 0.052 | 5.687% ± 0.073 | 10.732% ± 0.098 | 89.268% ± 0.098 |
| Strict JIT, Rule Box Ability lock, first | 4.731% ± 0.067 | 17.071% ± 0.119 | 29.032% ± 0.144 | 70.968% ± 0.144 |
| Strict JIT, combined lock, first | 0.475% ± 0.022 | 1.337% ± 0.036 | 3.098% ± 0.055 | 96.902% ± 0.055 |
| Strict JIT, going second | 46.468% ± 0.158 | 61.809% ± 0.154 | 72.549% ± 0.141 | 27.451% ± 0.141 |
| Matchup-flex JIT, going second | 49.155% ± 0.158 | 64.345% ± 0.151 | 74.369% ± 0.138 | 25.631% ± 0.138 |
| No discard control, going second | 58.697% ± 0.156 | 73.223% ± 0.140 | 81.759% ± 0.122 | 18.241% ± 0.122 |
| Strict JIT, turn-two Item lock, second | 6.612% ± 0.079 | 11.839% ± 0.102 | 17.549% ± 0.120 | 82.451% ± 0.120 |
| Strict JIT, full Item lock, second | 4.397% ± 0.065 | 9.128% ± 0.091 | 14.467% ± 0.111 | 85.533% ± 0.111 |
| Strict JIT, Rule Box Ability lock, second | 11.981% ± 0.103 | 23.380% ± 0.134 | 33.009% ± 0.149 | 66.991% ± 0.149 |
| Strict JIT, combined lock, second | 1.164% ± 0.034 | 2.610% ± 0.050 | 4.452% ± 0.065 | 95.548% ± 0.065 |
| Strict JIT, Supporter lock, first | 1.863% ± 0.043 | 5.949% ± 0.075 | 10.358% ± 0.096 | 89.642% ± 0.096 |
| Strict JIT, Supporter lock, second | 6.049% ± 0.075 | 10.055% ± 0.095 | 14.665% ± 0.112 | 85.335% ± 0.112 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.482% | 26.364% | 17.837% | 10.801% |
| Matchup-flex JIT, going first | 19.939% | 26.378% | 18.077% | 10.309% |
| No discard control, going first | 24.848% | 34.282% | 15.442% | 7.704% |
| Strict JIT, going second | 46.468% | 15.341% | 10.740% | 6.440% |
| Matchup-flex JIT, going second | 49.155% | 15.190% | 10.024% | 6.147% |
| No discard control, going second | 58.697% | 14.526% | 8.536% | 4.715% |

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
| Secret Box use | 63.686% |
| Exploding Energy use | 78.197% |
| Steven use | 35.983% |
| Star Alchemy use | 48.053% |
| Secret Box attempts | 1.517 per game |
| Cost blocks | 0.046 per game |
| Missing route axis | 0.833 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.276 per game |
| Steven banks | 0.303 per game |
| Gladion banks | 0.041 per game |
| FSS banks | 0.045 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.402 |
| Pineco/Forretress line | 0.443 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.005 |
| Fire | 0.031 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.159 |
| Known Prize zone | 0.496 |
| Discard zone | 0.251 |
| Stranded hand zone | 0.202 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `825d2edbe4a79a30fbce6506cb5dbd75b8fea48e6570e211d459b63b78bf0bb0`.

Comparison CSV SHA-256: `24d9565b22a5a13c6347e11eafcc74357f9928da9e4ddc72dacb0d1538acf7f8`.
