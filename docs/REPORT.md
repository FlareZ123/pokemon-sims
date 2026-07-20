# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.373% | 37.104% | 53.801% |
| Matchup-flex JIT, going first | 16.139% | 47.029% | 62.769% |
| No discard control, going first | 19.91% | 55.372% | 71.509% |
| Strict JIT, going second | 28.153% | 50.608% | 62.162% |
| Matchup-flex JIT, going second | 36.338% | 60.069% | 70.657% |
| No discard control, going second | 39.582% | 66.298% | 77.981% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.627% | 10.253% | 17.681% |
| Strict JIT, full Item lock, first | 2.772% | 7.448% | 14.718% |
| Strict JIT, Rule Box Ability lock, first | 4.299% | 25.556% | 38.51% |
| Strict JIT, combined lock, first | 0.288% | 3.257% | 7.149% |
| Strict JIT, turn-two Item lock, second | 13.586% | 26.928% | 34.443% |
| Strict JIT, full Item lock, second | 10.558% | 22.349% | 29.356% |
| Strict JIT, Rule Box Ability lock, second | 17.72% | 33.759% | 43.894% |
| Strict JIT, combined lock, second | 2.415% | 10.985% | 15.011% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
