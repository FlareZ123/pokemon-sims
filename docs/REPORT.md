# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.168% | 39.99% | 56.973% |
| Matchup-flex JIT, going first | 16.351% | 48.198% | 64.079% |
| No discard control, going first | 19.958% | 56.02% | 72.356% |
| Strict JIT, going second | 29.961% | 53.878% | 65.213% |
| Matchup-flex JIT, going second | 37.233% | 61.249% | 71.441% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.628% | 10.36% | 18.117% |
| Strict JIT, full Item lock, first | 2.872% | 7.914% | 15.402% |
| Strict JIT, Rule Box Ability lock, first | 4.461% | 26.487% | 40.107% |
| Strict JIT, combined lock, first | 0.304% | 3.356% | 7.482% |
| Strict JIT, turn-two Item lock, second | 14.186% | 28.279% | 36.247% |
| Strict JIT, full Item lock, second | 10.522% | 23.171% | 30.574% |
| Strict JIT, Rule Box Ability lock, second | 18.238% | 35.55% | 46.061% |
| Strict JIT, combined lock, second | 2.475% | 11.567% | 15.975% |
| Strict JIT, Supporter lock, first | 0.003% | 15.382% | 21.683% |
| Strict JIT, Supporter lock, second | 8.123% | 19.412% | 25.305% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
