# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.808% | 38.046% | 54.987% |
| Matchup-flex JIT, going first | 16.174% | 47.248% | 63.136% |
| No discard control, going first | 19.931% | 55.711% | 71.713% |
| Strict JIT, going second | 28.853% | 52.174% | 63.49% |
| Matchup-flex JIT, going second | 36.753% | 60.649% | 71.109% |
| No discard control, going second | 39.845% | 66.994% | 78.455% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.582% | 10.187% | 17.684% |
| Strict JIT, full Item lock, first | 2.82% | 7.733% | 15.056% |
| Strict JIT, Rule Box Ability lock, first | 4.36% | 25.688% | 38.826% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.27% |
| Strict JIT, turn-two Item lock, second | 14.187% | 28.035% | 35.673% |
| Strict JIT, full Item lock, second | 10.495% | 22.877% | 30.039% |
| Strict JIT, Rule Box Ability lock, second | 18.002% | 34.556% | 44.866% |
| Strict JIT, combined lock, second | 2.352% | 11.342% | 15.418% |
| Strict JIT, Supporter lock, first | 0.002% | 10.065% | 16.99% |
| Strict JIT, Supporter lock, second | 6.112% | 14.528% | 20.991% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
