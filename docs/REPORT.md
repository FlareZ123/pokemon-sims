# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.133% | 37.152% | 53.861% |
| Matchup-flex JIT, going first | 16.217% | 46.992% | 63.002% |
| No discard control, going first | 19.919% | 55.272% | 71.425% |
| Strict JIT, going second | 28.222% | 50.989% | 62.538% |
| Matchup-flex JIT, going second | 36.341% | 60.274% | 70.811% |
| No discard control, going second | 39.654% | 66.727% | 78.133% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.641% | 10.296% | 17.735% |
| Strict JIT, full Item lock, first | 2.785% | 7.514% | 14.761% |
| Strict JIT, Rule Box Ability lock, first | 4.328% | 25.467% | 38.469% |
| Strict JIT, combined lock, first | 0.285% | 3.262% | 7.17% |
| Strict JIT, turn-two Item lock, second | 13.783% | 27.046% | 34.459% |
| Strict JIT, full Item lock, second | 10.607% | 22.468% | 29.469% |
| Strict JIT, Rule Box Ability lock, second | 17.862% | 33.969% | 44.208% |
| Strict JIT, combined lock, second | 2.42% | 11.02% | 15.159% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
