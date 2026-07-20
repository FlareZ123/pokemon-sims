# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.338% | 37.173% | 53.733% |
| Matchup-flex JIT, going first | 16.077% | 46.81% | 62.551% |
| No discard control, going first | 19.941% | 55.418% | 71.503% |
| Strict JIT, going second | 28.322% | 50.646% | 62.216% |
| Matchup-flex JIT, going second | 36.311% | 59.625% | 70.269% |
| No discard control, going second | 39.768% | 66.516% | 78.116% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.601% | 10.289% | 17.766% |
| Strict JIT, full Item lock, first | 2.761% | 7.517% | 14.838% |
| Strict JIT, Rule Box Ability lock, first | 4.313% | 25.643% | 38.626% |
| Strict JIT, combined lock, first | 0.285% | 3.254% | 7.185% |
| Strict JIT, turn-two Item lock, second | 13.94% | 27.434% | 34.825% |
| Strict JIT, full Item lock, second | 10.752% | 22.553% | 29.485% |
| Strict JIT, Rule Box Ability lock, second | 17.85% | 33.841% | 43.963% |
| Strict JIT, combined lock, second | 2.434% | 10.992% | 15.024% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
