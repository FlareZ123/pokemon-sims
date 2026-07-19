# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.116% | 36.817% | 53.395% |
| Matchup-flex JIT, going first | 16.329% | 46.626% | 62.77% |
| No discard control, going first | 20.014% | 55.449% | 71.556% |
| Strict JIT, going second | 28.099% | 50.016% | 61.444% |
| Matchup-flex JIT, going second | 36.075% | 59.538% | 70.374% |
| No discard control, going second | 39.444% | 66.43% | 78.146% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.446% | 10.12% | 17.698% |
| Strict JIT, full Item lock, first | 2.753% | 7.632% | 14.765% |
| Strict JIT, Rule Box Ability lock, first | 4.453% | 25.073% | 37.895% |
| Strict JIT, combined lock, first | 0.322% | 3.335% | 7.288% |
| Strict JIT, turn-two Item lock, second | 13.786% | 27.028% | 34.456% |
| Strict JIT, full Item lock, second | 10.537% | 22.414% | 29.222% |
| Strict JIT, Rule Box Ability lock, second | 17.796% | 33.46% | 43.595% |
| Strict JIT, combined lock, second | 2.435% | 10.919% | 14.891% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
