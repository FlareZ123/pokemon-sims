# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.022% | 35.315% | 50.809% |
| Matchup-flex JIT, going first | 15.969% | 44.667% | 60.272% |
| No discard control, going first | 19.701% | 54.724% | 71.1% |
| Strict JIT, going second | 26.842% | 47.943% | 59.082% |
| Matchup-flex JIT, going second | 35.165% | 57.067% | 67.875% |
| No discard control, going second | 38.545% | 64.326% | 76.502% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.418% | 9.7% | 16.905% |
| Strict JIT, full Item lock, first | 2.771% | 7.279% | 14.18% |
| Strict JIT, Rule Box Ability lock, first | 4.167% | 24.007% | 36.447% |
| Strict JIT, combined lock, first | 0.317% | 3.226% | 7.173% |
| Strict JIT, turn-two Item lock, second | 13.339% | 26.365% | 33.597% |
| Strict JIT, full Item lock, second | 10.334% | 22.003% | 28.839% |
| Strict JIT, Rule Box Ability lock, second | 16.984% | 32.298% | 42.411% |
| Strict JIT, combined lock, second | 2.498% | 11.088% | 14.961% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
