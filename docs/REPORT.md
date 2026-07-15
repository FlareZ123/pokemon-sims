# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 9.983% | 29.907% | 42.362% |
| Matchup-flex JIT, going first | 13.229% | 35.949% | 49.389% |
| No discard control, going first | 15.353% | 50.66% | 65.432% |
| Strict JIT, going second | 24.444% | 42.818% | 51.857% |
| Matchup-flex JIT, going second | 31.457% | 50.113% | 58.698% |
| No discard control, going second | 34.893% | 60.191% | 70.829% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 3.627% | 7.659% | 11.89% |
| Strict JIT, full Item lock, first | 2.635% | 6.318% | 10.165% |
| Strict JIT, Rule Box Ability lock, first | 3.988% | 18.516% | 28.11% |
| Strict JIT, combined lock, first | 0.29% | 2.396% | 4.702% |
| Strict JIT, turn-two Item lock, second | 12.259% | 23.348% | 28.706% |
| Strict JIT, full Item lock, second | 9.79% | 19.499% | 24.598% |
| Strict JIT, Rule Box Ability lock, second | 16.116% | 27.746% | 34.877% |
| Strict JIT, combined lock, second | 2.329% | 9.399% | 11.786% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
