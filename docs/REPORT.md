# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.93% | 38.445% | 55.288% |
| Matchup-flex JIT, going first | 16.371% | 47.333% | 63.334% |
| No discard control, going first | 19.972% | 55.863% | 71.922% |
| Strict JIT, going second | 29.073% | 52.467% | 63.781% |
| Matchup-flex JIT, going second | 37.005% | 60.702% | 71.103% |
| No discard control, going second | 39.956% | 66.931% | 78.204% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.596% | 10.188% | 17.698% |
| Strict JIT, full Item lock, first | 2.825% | 7.752% | 15.07% |
| Strict JIT, Rule Box Ability lock, first | 4.332% | 25.881% | 38.859% |
| Strict JIT, combined lock, first | 0.291% | 3.268% | 7.254% |
| Strict JIT, turn-two Item lock, second | 14.081% | 27.971% | 35.63% |
| Strict JIT, full Item lock, second | 10.525% | 22.92% | 30.083% |
| Strict JIT, Rule Box Ability lock, second | 17.942% | 34.505% | 44.704% |
| Strict JIT, combined lock, second | 2.368% | 11.389% | 15.503% |
| Strict JIT, Supporter lock, first | 0.004% | 15.282% | 21.547% |
| Strict JIT, Supporter lock, second | 8.099% | 19.428% | 25.336% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
