# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.024% | 39.075% | 56.131% |
| Matchup-flex JIT, going first | 16.404% | 47.715% | 63.562% |
| No discard control, going first | 20.013% | 56.018% | 72.215% |
| Strict JIT, going second | 29.515% | 53.23% | 64.565% |
| Matchup-flex JIT, going second | 37.332% | 61.054% | 71.401% |
| No discard control, going second | 39.939% | 67.079% | 78.239% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.643% | 10.398% | 18.136% |
| Strict JIT, full Item lock, first | 2.753% | 7.685% | 15.072% |
| Strict JIT, Rule Box Ability lock, first | 4.344% | 26.386% | 39.701% |
| Strict JIT, combined lock, first | 0.293% | 3.309% | 7.394% |
| Strict JIT, turn-two Item lock, second | 14.262% | 28.249% | 35.905% |
| Strict JIT, full Item lock, second | 10.627% | 23.371% | 30.6% |
| Strict JIT, Rule Box Ability lock, second | 17.946% | 35.215% | 45.717% |
| Strict JIT, combined lock, second | 2.376% | 11.343% | 15.623% |
| Strict JIT, Supporter lock, first | 0.001% | 15.327% | 21.605% |
| Strict JIT, Supporter lock, second | 8.192% | 19.69% | 25.501% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
