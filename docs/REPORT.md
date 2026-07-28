# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.039% | 38.603% | 55.424% |
| Matchup-flex JIT, going first | 16.299% | 47.621% | 63.434% |
| No discard control, going first | 20.103% | 55.921% | 72.059% |
| Strict JIT, going second | 29.383% | 52.581% | 63.832% |
| Matchup-flex JIT, going second | 37.32% | 60.8% | 71.034% |
| No discard control, going second | 39.839% | 66.843% | 77.984% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.601% | 10.191% | 17.704% |
| Strict JIT, full Item lock, first | 2.825% | 7.752% | 15.07% |
| Strict JIT, Rule Box Ability lock, first | 4.441% | 25.965% | 39.028% |
| Strict JIT, combined lock, first | 0.287% | 3.278% | 7.27% |
| Strict JIT, turn-two Item lock, second | 14.086% | 27.941% | 35.593% |
| Strict JIT, full Item lock, second | 10.531% | 22.928% | 30.088% |
| Strict JIT, Rule Box Ability lock, second | 18.093% | 34.622% | 44.73% |
| Strict JIT, combined lock, second | 2.37% | 11.414% | 15.52% |
| Strict JIT, Supporter lock, first | 0.003% | 15.289% | 21.545% |
| Strict JIT, Supporter lock, second | 8.122% | 19.457% | 25.347% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
