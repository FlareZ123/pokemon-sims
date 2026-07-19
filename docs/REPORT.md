# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.878% | 35.724% | 51.269% |
| Matchup-flex JIT, going first | 15.947% | 44.791% | 60.686% |
| No discard control, going first | 19.734% | 54.79% | 71.068% |
| Strict JIT, going second | 26.853% | 48.373% | 59.394% |
| Matchup-flex JIT, going second | 34.878% | 57.947% | 68.559% |
| No discard control, going second | 39.453% | 66.012% | 77.716% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.364% | 9.633% | 17.008% |
| Strict JIT, full Item lock, first | 2.809% | 7.379% | 14.39% |
| Strict JIT, Rule Box Ability lock, first | 4.403% | 24.193% | 36.453% |
| Strict JIT, combined lock, first | 0.308% | 3.27% | 7.2% |
| Strict JIT, turn-two Item lock, second | 13.491% | 26.795% | 34.033% |
| Strict JIT, full Item lock, second | 10.407% | 22.17% | 29.132% |
| Strict JIT, Rule Box Ability lock, second | 17.18% | 32.324% | 42.461% |
| Strict JIT, combined lock, second | 2.51% | 11.049% | 15.047% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
