# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.209% | 37.418% | 54.161% |
| Matchup-flex JIT, going first | 16.236% | 46.964% | 63.02% |
| No discard control, going first | 19.919% | 55.272% | 71.425% |
| Strict JIT, going second | 28.027% | 51.045% | 62.772% |
| Matchup-flex JIT, going second | 36.305% | 60.279% | 70.815% |
| No discard control, going second | 39.686% | 66.751% | 78.152% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.645% | 10.311% | 17.772% |
| Strict JIT, full Item lock, first | 2.775% | 7.502% | 14.744% |
| Strict JIT, Rule Box Ability lock, first | 4.36% | 25.543% | 38.621% |
| Strict JIT, combined lock, first | 0.286% | 3.259% | 7.171% |
| Strict JIT, turn-two Item lock, second | 13.892% | 27.476% | 34.847% |
| Strict JIT, full Item lock, second | 10.71% | 23.002% | 29.928% |
| Strict JIT, Rule Box Ability lock, second | 17.815% | 34.227% | 44.326% |
| Strict JIT, combined lock, second | 2.39% | 11.417% | 15.485% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
