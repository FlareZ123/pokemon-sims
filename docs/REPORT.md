# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.321% | 35.174% | 50.526% |
| Matchup-flex JIT, going first | 15.964% | 43.898% | 59.596% |
| No discard control, going first | 19.623% | 54.848% | 71.281% |
| Strict JIT, going second | 27.685% | 48.843% | 59.386% |
| Matchup-flex JIT, going second | 35.35% | 57.329% | 67.947% |
| No discard control, going second | 38.688% | 64.507% | 76.541% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.343% | 9.416% | 16.56% |
| Strict JIT, full Item lock, first | 2.771% | 7.279% | 14.18% |
| Strict JIT, Rule Box Ability lock, first | 4.395% | 23.531% | 35.401% |
| Strict JIT, combined lock, first | 0.317% | 3.226% | 7.173% |
| Strict JIT, turn-two Item lock, second | 13.203% | 26.181% | 33.414% |
| Strict JIT, full Item lock, second | 10.334% | 22.003% | 28.839% |
| Strict JIT, Rule Box Ability lock, second | 17.411% | 32.004% | 41.385% |
| Strict JIT, combined lock, second | 2.498% | 11.088% | 14.961% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
