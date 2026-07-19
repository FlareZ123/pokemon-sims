# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.106% | 35.849% | 52.575% |
| Matchup-flex JIT, going first | 16.22% | 45.65% | 62.106% |
| No discard control, going first | 19.727% | 54.858% | 71.372% |
| Strict JIT, going second | 27.038% | 48.942% | 60.77% |
| Matchup-flex JIT, going second | 35.128% | 58.789% | 69.807% |
| No discard control, going second | 39.175% | 66.302% | 77.92% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.573% | 10.128% | 17.673% |
| Strict JIT, full Item lock, first | 2.759% | 7.55% | 14.83% |
| Strict JIT, Rule Box Ability lock, first | 4.275% | 24.569% | 37.105% |
| Strict JIT, combined lock, first | 0.315% | 3.264% | 7.198% |
| Strict JIT, turn-two Item lock, second | 13.501% | 26.947% | 34.325% |
| Strict JIT, full Item lock, second | 10.497% | 22.305% | 29.32% |
| Strict JIT, Rule Box Ability lock, second | 17.346% | 32.589% | 43.008% |
| Strict JIT, combined lock, second | 2.517% | 11.073% | 15.098% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
