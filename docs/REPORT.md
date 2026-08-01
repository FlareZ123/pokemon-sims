# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.139% | 39.866% | 56.768% |
| Matchup-flex JIT, going first | 16.501% | 48.32% | 64.072% |
| No discard control, going first | 19.956% | 56.021% | 72.357% |
| Strict JIT, going second | 30.085% | 53.973% | 65.288% |
| Matchup-flex JIT, going second | 37.317% | 61.368% | 71.482% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.551% | 10.285% | 18.044% |
| Strict JIT, full Item lock, first | 2.839% | 7.771% | 15.26% |
| Strict JIT, Rule Box Ability lock, first | 4.409% | 26.589% | 40.182% |
| Strict JIT, combined lock, first | 0.313% | 3.357% | 7.468% |
| Strict JIT, turn-two Item lock, second | 14.204% | 28.109% | 35.87% |
| Strict JIT, full Item lock, second | 10.565% | 23.255% | 30.566% |
| Strict JIT, Rule Box Ability lock, second | 18.231% | 35.518% | 46.051% |
| Strict JIT, combined lock, second | 2.514% | 11.507% | 15.879% |
| Strict JIT, Supporter lock, first | 0.003% | 15.466% | 21.733% |
| Strict JIT, Supporter lock, second | 8.152% | 19.442% | 25.322% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
