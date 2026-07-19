# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.178% | 35.755% | 52.31% |
| Matchup-flex JIT, going first | 15.964% | 45.418% | 61.593% |
| No discard control, going first | 19.768% | 54.842% | 70.959% |
| Strict JIT, going second | 26.942% | 48.955% | 60.729% |
| Matchup-flex JIT, going second | 35.076% | 58.642% | 69.814% |
| No discard control, going second | 39.452% | 65.973% | 77.66% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.526% | 10.053% | 17.38% |
| Strict JIT, full Item lock, first | 2.824% | 7.481% | 14.493% |
| Strict JIT, Rule Box Ability lock, first | 4.332% | 24.651% | 37.208% |
| Strict JIT, combined lock, first | 0.31% | 3.266% | 7.19% |
| Strict JIT, turn-two Item lock, second | 13.482% | 26.871% | 34.195% |
| Strict JIT, full Item lock, second | 10.448% | 22.225% | 29.194% |
| Strict JIT, Rule Box Ability lock, second | 17.38% | 32.607% | 43.063% |
| Strict JIT, combined lock, second | 2.506% | 11.048% | 15.045% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
