# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.343% | 37.19% | 53.744% |
| Matchup-flex JIT, going first | 16.118% | 46.823% | 62.591% |
| No discard control, going first | 19.941% | 55.418% | 71.503% |
| Strict JIT, going second | 28.34% | 50.626% | 62.105% |
| Matchup-flex JIT, going second | 36.306% | 59.701% | 70.329% |
| No discard control, going second | 39.77% | 66.509% | 78.108% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.602% | 10.286% | 17.761% |
| Strict JIT, full Item lock, first | 2.757% | 7.509% | 14.827% |
| Strict JIT, Rule Box Ability lock, first | 4.317% | 25.607% | 38.597% |
| Strict JIT, combined lock, first | 0.287% | 3.255% | 7.187% |
| Strict JIT, turn-two Item lock, second | 13.964% | 27.437% | 34.829% |
| Strict JIT, full Item lock, second | 10.731% | 22.539% | 29.478% |
| Strict JIT, Rule Box Ability lock, second | 17.842% | 33.829% | 43.959% |
| Strict JIT, combined lock, second | 2.434% | 10.993% | 15.026% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
