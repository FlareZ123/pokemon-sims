# Named-Deck Setup Comparison

This report is generated from [`../results/multi_deck_comparison.csv`](../results/multi_deck_comparison.csv) and [`../results/multi_deck_manifest.json`](../results/multi_deck_manifest.json).

Fixed seed: `20260705`. Trials per condition: `100,000`. Conditions: `32`. Total simulated games: `3,200,000`.

Both decks use the same derived seed for each scenario. This common-random-number design reduces comparison noise while preserving the historical shell seed schedule. `regidrago-shell` remains the default when `--deck` is omitted. `regidrago-pineco` is the Secret Box recipe with Pineco, Forretress ex, Dawn, Forest of Vitality, and Appletun `sv8-140`. The withdrawn Pineco Brilliant Blender variant is absent from the registry and results.

## Direct comparison

| Scenario | Shell T2 | Pineco T2 | Δ T2 | Shell T3 | Pineco T3 | Δ T3 | Shell T4 | Pineco T4 | Δ T4 |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Strict JIT, going first | 11.740% | 18.481% | +6.741 pp | 38.005% | 44.282% | +6.277 pp | 55.157% | 61.772% | +6.615 pp |
| Strict JIT, going second | 28.955% | 46.056% | +17.101 pp | 52.147% | 60.930% | +8.783 pp | 63.452% | 71.466% | +8.014 pp |
| Matchup-flex JIT, going first | 16.241% | 19.928% | +3.687 pp | 47.208% | 45.492% | -1.716 pp | 63.001% | 63.308% | +0.307 pp |
| Matchup-flex JIT, going second | 36.984% | 48.803% | +11.819 pp | 60.679% | 64.008% | +3.329 pp | 71.015% | 73.720% | +2.705 pp |
| No discard control, going first | 19.936% | 24.669% | +4.733 pp | 55.391% | 58.065% | +2.674 pp | 71.503% | 73.801% | +2.298 pp |
| No discard control, going second | 39.911% | 58.419% | +18.508 pp | 66.972% | 72.872% | +5.900 pp | 78.508% | 81.342% | +2.834 pp |

## Regidrago shell

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.740% ± 0.102 | 38.005% ± 0.153 | 55.157% ± 0.157 | 44.843% ± 0.157 |
| Matchup-flex JIT, going first | 16.241% ± 0.117 | 47.208% ± 0.158 | 63.001% ± 0.153 | 36.999% ± 0.153 |
| No discard control, going first | 19.936% ± 0.126 | 55.391% ± 0.157 | 71.503% ± 0.143 | 28.497% ± 0.143 |
| Strict JIT, turn-two Item lock, first | 4.583% ± 0.066 | 10.183% ± 0.096 | 17.696% ± 0.121 | 82.304% ± 0.121 |
| Strict JIT, full Item lock, first | 2.821% ± 0.052 | 7.736% ± 0.084 | 15.064% ± 0.113 | 84.936% ± 0.113 |
| Strict JIT, Rule Box Ability lock, first | 4.387% ± 0.065 | 25.685% ± 0.138 | 38.800% ± 0.154 | 61.200% ± 0.154 |
| Strict JIT, combined lock, first | 0.293% ± 0.017 | 3.279% ± 0.056 | 7.270% ± 0.082 | 92.730% ± 0.082 |
| Strict JIT, going second | 28.955% ± 0.143 | 52.147% ± 0.158 | 63.452% ± 0.152 | 36.548% ± 0.152 |
| Matchup-flex JIT, going second | 36.984% ± 0.153 | 60.679% ± 0.154 | 71.015% ± 0.143 | 28.985% ± 0.143 |
| No discard control, going second | 39.911% ± 0.155 | 66.972% ± 0.149 | 78.508% ± 0.130 | 21.492% ± 0.130 |
| Strict JIT, turn-two Item lock, second | 14.201% ± 0.110 | 28.029% ± 0.142 | 35.672% ± 0.151 | 64.328% ± 0.151 |
| Strict JIT, full Item lock, second | 10.493% ± 0.097 | 22.882% ± 0.133 | 30.032% ± 0.145 | 69.968% ± 0.145 |
| Strict JIT, Rule Box Ability lock, second | 18.138% ± 0.122 | 34.559% ± 0.150 | 44.875% ± 0.157 | 55.125% ± 0.157 |
| Strict JIT, combined lock, second | 2.352% ± 0.048 | 11.342% ± 0.100 | 15.418% ± 0.114 | 84.582% ± 0.114 |
| Strict JIT, Supporter lock, first | 0.002% ± 0.001 | 10.065% ± 0.095 | 16.990% ± 0.119 | 83.010% ± 0.119 |
| Strict JIT, Supporter lock, second | 6.112% ± 0.076 | 14.528% ± 0.111 | 20.991% ± 0.129 | 79.009% ± 0.129 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 11.740% | 26.265% | 17.152% | 10.760% |
| Matchup-flex JIT, going first | 16.241% | 30.967% | 15.793% | 9.776% |
| No discard control, going first | 19.936% | 35.455% | 16.112% | 9.051% |
| Strict JIT, going second | 28.955% | 23.192% | 11.305% | 7.893% |
| Matchup-flex JIT, going second | 36.984% | 23.695% | 10.336% | 7.127% |
| No discard control, going second | 39.911% | 27.061% | 11.536% | 6.539% |

## Regidrago-Pineco with Secret Box

| Scenario | T2 ± SE | T3 ± SE | T4 ± SE | Failure ± SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.481% ± 0.123 | 44.282% ± 0.157 | 61.772% ± 0.154 | 38.228% ± 0.154 |
| Matchup-flex JIT, going first | 19.928% ± 0.126 | 45.492% ± 0.157 | 63.308% ± 0.152 | 36.692% ± 0.152 |
| No discard control, going first | 24.669% ± 0.136 | 58.065% ± 0.156 | 73.801% ± 0.139 | 26.199% ± 0.139 |
| Strict JIT, turn-two Item lock, first | 4.653% ± 0.067 | 7.988% ± 0.086 | 13.611% ± 0.108 | 86.389% ± 0.108 |
| Strict JIT, full Item lock, first | 2.784% ± 0.052 | 5.742% ± 0.074 | 11.124% ± 0.099 | 88.876% ± 0.099 |
| Strict JIT, Rule Box Ability lock, first | 5.341% ± 0.071 | 19.239% ± 0.125 | 31.981% ± 0.147 | 68.019% ± 0.147 |
| Strict JIT, combined lock, first | 0.634% ± 0.025 | 1.863% ± 0.043 | 4.300% ± 0.064 | 95.700% ± 0.064 |
| Strict JIT, going second | 46.056% ± 0.158 | 60.930% ± 0.154 | 71.466% ± 0.143 | 28.534% ± 0.143 |
| Matchup-flex JIT, going second | 48.803% ± 0.158 | 64.008% ± 0.152 | 73.720% ± 0.139 | 26.280% ± 0.139 |
| No discard control, going second | 58.419% ± 0.156 | 72.872% ± 0.141 | 81.342% ± 0.123 | 18.658% ± 0.123 |
| Strict JIT, turn-two Item lock, second | 6.704% ± 0.079 | 12.110% ± 0.103 | 17.907% ± 0.121 | 82.093% ± 0.121 |
| Strict JIT, full Item lock, second | 4.705% ± 0.067 | 9.544% ± 0.093 | 15.105% ± 0.113 | 84.895% ± 0.113 |
| Strict JIT, Rule Box Ability lock, second | 13.197% ± 0.107 | 25.754% ± 0.138 | 36.259% ± 0.152 | 63.741% ± 0.152 |
| Strict JIT, combined lock, second | 1.374% ± 0.037 | 3.302% ± 0.057 | 5.574% ± 0.073 | 94.426% ± 0.073 |
| Strict JIT, Supporter lock, first | 1.113% ± 0.033 | 5.968% ± 0.075 | 10.117% ± 0.095 | 89.883% ± 0.095 |
| Strict JIT, Supporter lock, second | 4.966% ± 0.069 | 9.656% ± 0.093 | 13.972% ± 0.110 | 86.028% ± 0.110 |

### First-ready-turn distribution

| Scenario | Ready on T2 | Ready on T3 | Ready on T4 | Ready on T5 diagnostic |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 18.481% | 25.801% | 17.490% | 10.635% |
| Matchup-flex JIT, going first | 19.928% | 25.564% | 17.816% | 10.503% |
| No discard control, going first | 24.669% | 33.396% | 15.736% | 7.879% |
| Strict JIT, going second | 46.056% | 14.874% | 10.536% | 6.623% |
| Matchup-flex JIT, going second | 48.803% | 15.205% | 9.712% | 6.308% |
| No discard control, going second | 58.419% | 14.453% | 8.470% | 4.981% |

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
| Secret Box use | 62.503% |
| Exploding Energy use | 82.959% |
| Steven use | 36.537% |
| Star Alchemy use | 46.162% |
| Secret Box attempts | 1.559 per game |
| Cost blocks | 0.051 per game |
| Missing route axis | 0.882 per game |
| Bench blocks | 0.001 per game |
| Arven banks | 0.274 per game |
| Steven banks | 0.308 per game |
| Gladion banks | 0.039 per game |
| FSS banks | 0.044 per game |

### Overlapping axis and zone counters

| Overlapping failure reason | Events per game |
|---|---:|
| Regidrago line | 0.395 |
| Pineco/Forretress line | 0.493 |
| VSTAR | 0.005 |
| Payload | 0.000 |
| Search Item | 0.004 |
| Fire | 0.031 |
| Grass | 0.002 |
| Ability | 0.000 |
| Supporter | 0.170 |
| Known Prize zone | 0.528 |
| Discard zone | 0.297 |
| Stranded hand zone | 0.179 |

## Why more Basics did not guarantee a faster deck

Four Tapu Lele-GX and two Pineco reduce mulligans, while they also lower the probability of opening Regidrago V. Active Tapu or Pineco can require a retreat or switch plan, the six support Pokémon compete for Bench space, and partially drawn Pineco pieces have value only when the full route can execute. Secret Box requires three other hand cards, while Mysterious Treasure may require another cost. Prizing can collapse the ACE SPEC, Forest, Forest Seal Stone, Pineco line, VSTAR, Fire channel, or connector path. Item, Supporter, and Rule Box Ability locks remove different parts of the chain.

The corrected planner distinguishes sequential Supporters from same-turn contention. A planned T1 Arven, Gladion, or Steven play can be followed by T2 Dawn. It also compares direct shell-style completion against the Pineco route before committing resources.

## Boundary

These percentages estimate setup readiness under the documented goldfish policy. They are not match-win rates. The model does not assign strategic value to giving up two Prizes through Exploding Energy, repeated attacks, opponent damage, gust, hand disruption, or full format legality.

## Provenance

Simulator policy digest: `d2ff7474c63f30759953c89ca37bcba611cbb16b60264bec996954a93f24f3f3`.

Comparison CSV SHA-256: `cd12de3f11696bf5d71967484958e41105a54c3c7b9d29412bf3f2ab783a7334`.
