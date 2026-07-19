# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.738% | 34.892% | 50.454% |
| Matchup-flex JIT, going first | 16.061% | 44.576% | 60.222% |
| No discard control, going first | 19.701% | 54.724% | 71.1% |
| Strict JIT, going second | 26.773% | 48.066% | 59.086% |
| Matchup-flex JIT, going second | 34.915% | 57.257% | 68.147% |
| No discard control, going second | 38.508% | 64.317% | 76.499% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.418% | 9.7% | 16.905% |
| Strict JIT, full Item lock, first | 2.771% | 7.279% | 14.18% |
| Strict JIT, Rule Box Ability lock, first | 4.167% | 23.592% | 35.846% |
| Strict JIT, combined lock, first | 0.317% | 3.226% | 7.173% |
| Strict JIT, turn-two Item lock, second | 13.306% | 26.335% | 33.574% |
| Strict JIT, full Item lock, second | 10.334% | 22.003% | 28.839% |
| Strict JIT, Rule Box Ability lock, second | 16.78% | 31.906% | 41.866% |
| Strict JIT, combined lock, second | 2.498% | 11.088% | 14.961% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
