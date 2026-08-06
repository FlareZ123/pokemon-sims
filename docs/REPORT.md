# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.06% | 40.047% | 56.973% |
| Matchup-flex JIT, going first | 16.327% | 48.599% | 64.979% |
| No discard control, going first | 19.958% | 56.02% | 72.356% |
| Strict JIT, going second | 29.873% | 53.743% | 65.287% |
| Matchup-flex JIT, going second | 37.164% | 61.518% | 72.143% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.601% | 10.393% | 18.262% |
| Strict JIT, full Item lock, first | 2.85% | 7.89% | 15.425% |
| Strict JIT, Rule Box Ability lock, first | 4.408% | 26.53% | 40.311% |
| Strict JIT, combined lock, first | 0.308% | 3.371% | 7.514% |
| Strict JIT, turn-two Item lock, second | 14.177% | 28.393% | 36.916% |
| Strict JIT, full Item lock, second | 10.515% | 23.371% | 31.33% |
| Strict JIT, Rule Box Ability lock, second | 18.299% | 35.535% | 46.34% |
| Strict JIT, combined lock, second | 2.51% | 11.587% | 16.268% |
| Strict JIT, Supporter lock, first | 0.003% | 15.422% | 21.773% |
| Strict JIT, Supporter lock, second | 8.12% | 19.416% | 25.348% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
