# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.885% | 38.731% | 55.812% |
| Matchup-flex JIT, going first | 16.382% | 47.638% | 63.393% |
| No discard control, going first | 20.09% | 55.965% | 72.284% |
| Strict JIT, going second | 29.34% | 52.609% | 63.963% |
| Matchup-flex JIT, going second | 37.288% | 61.121% | 71.4% |
| No discard control, going second | 39.873% | 67.048% | 78.156% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.579% | 10.127% | 17.708% |
| Strict JIT, full Item lock, first | 2.789% | 7.658% | 15.045% |
| Strict JIT, Rule Box Ability lock, first | 4.372% | 26.093% | 39.266% |
| Strict JIT, combined lock, first | 0.278% | 3.231% | 7.225% |
| Strict JIT, turn-two Item lock, second | 14.027% | 27.939% | 35.743% |
| Strict JIT, full Item lock, second | 10.46% | 22.819% | 29.995% |
| Strict JIT, Rule Box Ability lock, second | 18.144% | 34.767% | 45.178% |
| Strict JIT, combined lock, second | 2.33% | 11.453% | 15.595% |
| Strict JIT, Supporter lock, first | 0.001% | 15.327% | 21.605% |
| Strict JIT, Supporter lock, second | 8.192% | 19.69% | 25.501% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
