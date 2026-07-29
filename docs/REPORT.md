# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.051% | 39.286% | 56.515% |
| Matchup-flex JIT, going first | 16.319% | 47.568% | 63.42% |
| No discard control, going first | 20.013% | 56.018% | 72.215% |
| Strict JIT, going second | 29.777% | 53.497% | 64.747% |
| Matchup-flex JIT, going second | 37.377% | 61.088% | 71.458% |
| No discard control, going second | 39.944% | 67.09% | 78.237% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.632% | 10.38% | 18.077% |
| Strict JIT, full Item lock, first | 2.752% | 7.683% | 15.067% |
| Strict JIT, Rule Box Ability lock, first | 4.403% | 26.487% | 39.76% |
| Strict JIT, combined lock, first | 0.293% | 3.309% | 7.394% |
| Strict JIT, turn-two Item lock, second | 14.297% | 28.22% | 35.805% |
| Strict JIT, full Item lock, second | 10.632% | 23.384% | 30.611% |
| Strict JIT, Rule Box Ability lock, second | 18.055% | 35.108% | 45.504% |
| Strict JIT, combined lock, second | 2.377% | 11.343% | 15.621% |
| Strict JIT, Supporter lock, first | 0.001% | 15.369% | 21.653% |
| Strict JIT, Supporter lock, second | 8.208% | 19.692% | 25.509% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
