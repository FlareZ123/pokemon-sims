# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.969% | 38% | 54.941% |
| Matchup-flex JIT, going first | 16.406% | 47.384% | 63.398% |
| No discard control, going first | 19.954% | 55.774% | 71.845% |
| Strict JIT, going second | 28.729% | 51.976% | 63.346% |
| Matchup-flex JIT, going second | 37.104% | 60.856% | 71.247% |
| No discard control, going second | 39.987% | 66.854% | 78.162% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.582% | 10.187% | 17.684% |
| Strict JIT, full Item lock, first | 2.82% | 7.733% | 15.056% |
| Strict JIT, Rule Box Ability lock, first | 4.463% | 25.882% | 38.851% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.27% |
| Strict JIT, turn-two Item lock, second | 14.086% | 27.977% | 35.638% |
| Strict JIT, full Item lock, second | 10.488% | 22.888% | 30.049% |
| Strict JIT, Rule Box Ability lock, second | 17.968% | 34.458% | 44.6% |
| Strict JIT, combined lock, second | 2.352% | 11.342% | 15.418% |
| Strict JIT, Supporter lock, first | 0.002% | 10.163% | 16.908% |
| Strict JIT, Supporter lock, second | 6.159% | 14.67% | 20.978% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
