# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.34% | 37.676% | 54.619% |
| Matchup-flex JIT, going first | 16.292% | 47.246% | 63.185% |
| No discard control, going first | 19.963% | 55.349% | 71.524% |
| Strict JIT, going second | 28.192% | 51.503% | 63.064% |
| Matchup-flex JIT, going second | 36.544% | 60.489% | 70.908% |
| No discard control, going second | 39.949% | 67% | 78.492% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.614% | 10.317% | 17.743% |
| Strict JIT, full Item lock, first | 2.777% | 7.457% | 14.725% |
| Strict JIT, Rule Box Ability lock, first | 4.456% | 25.613% | 38.69% |
| Strict JIT, combined lock, first | 0.282% | 3.243% | 7.154% |
| Strict JIT, turn-two Item lock, second | 13.864% | 27.435% | 35.054% |
| Strict JIT, full Item lock, second | 10.606% | 22.843% | 30.026% |
| Strict JIT, Rule Box Ability lock, second | 17.89% | 34.398% | 44.553% |
| Strict JIT, combined lock, second | 2.36% | 11.398% | 15.487% |
| Strict JIT, Supporter lock, first | 0.003% | 10.062% | 16.692% |
| Strict JIT, Supporter lock, second | 5.83% | 14.094% | 20.166% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
