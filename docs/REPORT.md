# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.02% | 38.433% | 55.371% |
| Matchup-flex JIT, going first | 16.406% | 47.384% | 63.398% |
| No discard control, going first | 19.954% | 55.774% | 71.845% |
| Strict JIT, going second | 28.953% | 52.211% | 63.523% |
| Matchup-flex JIT, going second | 37.104% | 60.856% | 71.247% |
| No discard control, going second | 39.987% | 66.854% | 78.162% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.582% | 10.187% | 17.684% |
| Strict JIT, full Item lock, first | 2.82% | 7.733% | 15.056% |
| Strict JIT, Rule Box Ability lock, first | 4.448% | 25.895% | 38.818% |
| Strict JIT, combined lock, first | 0.292% | 3.275% | 7.263% |
| Strict JIT, turn-two Item lock, second | 14.086% | 27.977% | 35.638% |
| Strict JIT, full Item lock, second | 10.488% | 22.888% | 30.049% |
| Strict JIT, Rule Box Ability lock, second | 17.979% | 34.485% | 44.591% |
| Strict JIT, combined lock, second | 2.361% | 11.368% | 15.466% |
| Strict JIT, Supporter lock, first | 0.004% | 15.257% | 21.516% |
| Strict JIT, Supporter lock, second | 8.085% | 19.364% | 25.257% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
