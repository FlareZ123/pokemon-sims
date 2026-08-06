# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.08% | 40.022% | 57.091% |
| Matchup-flex JIT, going first | 16.356% | 48.329% | 64.502% |
| No discard control, going first | 19.958% | 56.02% | 72.356% |
| Strict JIT, going second | 30.064% | 54.111% | 65.549% |
| Matchup-flex JIT, going second | 37.395% | 61.52% | 72.036% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.601% | 10.393% | 18.262% |
| Strict JIT, full Item lock, first | 2.85% | 7.89% | 15.425% |
| Strict JIT, Rule Box Ability lock, first | 4.437% | 26.578% | 40.334% |
| Strict JIT, combined lock, first | 0.308% | 3.371% | 7.514% |
| Strict JIT, turn-two Item lock, second | 14.177% | 28.393% | 36.916% |
| Strict JIT, full Item lock, second | 10.515% | 23.371% | 31.33% |
| Strict JIT, Rule Box Ability lock, second | 18.22% | 35.529% | 46.248% |
| Strict JIT, combined lock, second | 2.51% | 11.587% | 16.268% |
| Strict JIT, Supporter lock, first | 0.003% | 15.382% | 21.683% |
| Strict JIT, Supporter lock, second | 8.123% | 19.412% | 25.305% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
