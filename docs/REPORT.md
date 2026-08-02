# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.177% | 39.917% | 56.936% |
| Matchup-flex JIT, going first | 16.33% | 48.276% | 64.033% |
| No discard control, going first | 19.958% | 56.02% | 72.356% |
| Strict JIT, going second | 29.971% | 53.865% | 65.198% |
| Matchup-flex JIT, going second | 37.331% | 61.331% | 71.488% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.59% | 10.308% | 18.093% |
| Strict JIT, full Item lock, first | 2.869% | 7.902% | 15.391% |
| Strict JIT, Rule Box Ability lock, first | 4.458% | 26.437% | 40.09% |
| Strict JIT, combined lock, first | 0.304% | 3.357% | 7.499% |
| Strict JIT, turn-two Item lock, second | 14.138% | 28.125% | 35.922% |
| Strict JIT, full Item lock, second | 10.552% | 23.177% | 30.515% |
| Strict JIT, Rule Box Ability lock, second | 18.196% | 35.502% | 45.988% |
| Strict JIT, combined lock, second | 2.481% | 11.482% | 15.888% |
| Strict JIT, Supporter lock, first | 0.003% | 15.382% | 21.683% |
| Strict JIT, Supporter lock, second | 8.123% | 19.412% | 25.305% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
