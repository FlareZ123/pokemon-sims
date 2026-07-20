# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.369% | 37.131% | 53.796% |
| Matchup-flex JIT, going first | 16.232% | 47.005% | 62.782% |
| No discard control, going first | 19.93% | 55.332% | 71.457% |
| Strict JIT, going second | 28.211% | 50.297% | 61.843% |
| Matchup-flex JIT, going second | 36.106% | 59.473% | 70.218% |
| No discard control, going second | 39.564% | 66.469% | 78.063% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.622% | 10.298% | 17.719% |
| Strict JIT, full Item lock, first | 2.759% | 7.488% | 14.743% |
| Strict JIT, Rule Box Ability lock, first | 4.308% | 25.589% | 38.627% |
| Strict JIT, combined lock, first | 0.287% | 3.253% | 7.177% |
| Strict JIT, turn-two Item lock, second | 13.839% | 27.175% | 34.552% |
| Strict JIT, full Item lock, second | 10.638% | 22.357% | 29.287% |
| Strict JIT, Rule Box Ability lock, second | 17.811% | 33.758% | 43.898% |
| Strict JIT, combined lock, second | 2.434% | 10.966% | 14.996% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
