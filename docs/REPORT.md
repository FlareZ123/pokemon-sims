# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.084% | 36.691% | 53.3% |
| Matchup-flex JIT, going first | 16.293% | 46.627% | 62.625% |
| No discard control, going first | 20.014% | 55.449% | 71.556% |
| Strict JIT, going second | 28.08% | 50.071% | 61.63% |
| Matchup-flex JIT, going second | 36.008% | 59.478% | 70.179% |
| No discard control, going second | 39.444% | 66.43% | 78.146% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.507% | 10.126% | 17.696% |
| Strict JIT, full Item lock, first | 2.729% | 7.607% | 14.79% |
| Strict JIT, Rule Box Ability lock, first | 4.474% | 25.185% | 38.074% |
| Strict JIT, combined lock, first | 0.309% | 3.267% | 7.247% |
| Strict JIT, turn-two Item lock, second | 13.621% | 27.091% | 34.468% |
| Strict JIT, full Item lock, second | 10.422% | 22.228% | 29.165% |
| Strict JIT, Rule Box Ability lock, second | 17.665% | 33.342% | 43.524% |
| Strict JIT, combined lock, second | 2.5% | 11.057% | 15.076% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
