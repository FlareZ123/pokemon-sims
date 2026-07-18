# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.497% | 35.244% | 50.407% |
| Matchup-flex JIT, going first | 15.9% | 43.779% | 59.298% |
| No discard control, going first | 19.771% | 54.852% | 71.291% |
| Strict JIT, going second | 26.941% | 47.76% | 58.298% |
| Matchup-flex JIT, going second | 34.165% | 56.526% | 67.267% |
| No discard control, going second | 37.343% | 64.089% | 76.104% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.535% | 9.628% | 16.645% |
| Strict JIT, full Item lock, first | 2.881% | 7.429% | 14.131% |
| Strict JIT, Rule Box Ability lock, first | 4.415% | 23.64% | 35.361% |
| Strict JIT, combined lock, first | 0.274% | 3.245% | 7.079% |
| Strict JIT, turn-two Item lock, second | 13.19% | 25.627% | 32.806% |
| Strict JIT, full Item lock, second | 10.245% | 21.469% | 28.228% |
| Strict JIT, Rule Box Ability lock, second | 17.312% | 31.72% | 40.911% |
| Strict JIT, combined lock, second | 2.442% | 10.769% | 14.626% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
