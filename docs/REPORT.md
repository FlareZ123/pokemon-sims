# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.496% | 35.229% | 50.257% |
| Matchup-flex JIT, going first | 15.773% | 43.379% | 58.981% |
| No discard control, going first | 19.421% | 53.945% | 70.164% |
| Strict JIT, going second | 26.864% | 47.734% | 58.264% |
| Matchup-flex JIT, going second | 33.702% | 56.101% | 66.656% |
| No discard control, going second | 37.329% | 63.522% | 75.619% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.501% | 9.621% | 16.593% |
| Strict JIT, full Item lock, first | 2.868% | 7.446% | 14.028% |
| Strict JIT, Rule Box Ability lock, first | 4.443% | 23.515% | 35.391% |
| Strict JIT, combined lock, first | 0.292% | 3.239% | 7.064% |
| Strict JIT, turn-two Item lock, second | 13.072% | 25.673% | 32.563% |
| Strict JIT, full Item lock, second | 10.21% | 21.442% | 28.03% |
| Strict JIT, Rule Box Ability lock, second | 17.4% | 31.756% | 40.98% |
| Strict JIT, combined lock, second | 2.414% | 10.789% | 14.404% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
