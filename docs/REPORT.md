# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.9% | 35.584% | 51.284% |
| Matchup-flex JIT, going first | 16.146% | 44.96% | 60.483% |
| No discard control, going first | 19.67% | 54.679% | 70.962% |
| Strict JIT, going second | 26.753% | 47.88% | 58.971% |
| Matchup-flex JIT, going second | 35.529% | 57.47% | 68.32% |
| No discard control, going second | 38.614% | 64.499% | 76.464% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.349% | 9.562% | 16.92% |
| Strict JIT, full Item lock, first | 2.832% | 7.354% | 14.349% |
| Strict JIT, Rule Box Ability lock, first | 4.376% | 24.14% | 36.411% |
| Strict JIT, combined lock, first | 0.309% | 3.262% | 7.187% |
| Strict JIT, turn-two Item lock, second | 13.592% | 26.623% | 33.883% |
| Strict JIT, full Item lock, second | 10.371% | 22.145% | 29.106% |
| Strict JIT, Rule Box Ability lock, second | 17.288% | 32.38% | 42.499% |
| Strict JIT, combined lock, second | 2.507% | 11.06% | 15.036% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
