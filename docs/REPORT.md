# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.276% | 37.164% | 53.866% |
| Matchup-flex JIT, going first | 16.264% | 47.12% | 62.96% |
| No discard control, going first | 19.919% | 55.272% | 71.425% |
| Strict JIT, going second | 28.262% | 50.876% | 62.429% |
| Matchup-flex JIT, going second | 36.537% | 60.268% | 70.74% |
| No discard control, going second | 39.654% | 66.727% | 78.133% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.631% | 10.26% | 17.696% |
| Strict JIT, full Item lock, first | 2.776% | 7.456% | 14.729% |
| Strict JIT, Rule Box Ability lock, first | 4.328% | 25.467% | 38.469% |
| Strict JIT, combined lock, first | 0.286% | 3.262% | 7.155% |
| Strict JIT, turn-two Item lock, second | 13.593% | 26.917% | 34.431% |
| Strict JIT, full Item lock, second | 10.571% | 22.357% | 29.358% |
| Strict JIT, Rule Box Ability lock, second | 17.862% | 33.969% | 44.208% |
| Strict JIT, combined lock, second | 2.415% | 10.983% | 15.008% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
