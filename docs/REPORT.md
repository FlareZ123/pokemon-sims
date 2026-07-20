# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.358% | 37.123% | 53.792% |
| Matchup-flex JIT, going first | 16.146% | 47.011% | 62.763% |
| No discard control, going first | 19.91% | 55.372% | 71.509% |
| Strict JIT, going second | 28.341% | 50.83% | 62.416% |
| Matchup-flex JIT, going second | 36.293% | 60.024% | 70.571% |
| No discard control, going second | 39.563% | 66.301% | 77.997% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.631% | 10.259% | 17.695% |
| Strict JIT, full Item lock, first | 2.776% | 7.456% | 14.729% |
| Strict JIT, Rule Box Ability lock, first | 4.293% | 25.571% | 38.517% |
| Strict JIT, combined lock, first | 0.286% | 3.262% | 7.155% |
| Strict JIT, turn-two Item lock, second | 13.58% | 26.901% | 34.419% |
| Strict JIT, full Item lock, second | 10.571% | 22.357% | 29.358% |
| Strict JIT, Rule Box Ability lock, second | 17.733% | 33.779% | 43.915% |
| Strict JIT, combined lock, second | 2.415% | 10.983% | 15.008% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
