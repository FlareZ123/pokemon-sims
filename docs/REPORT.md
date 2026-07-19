# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.139% | 35.446% | 51.435% |
| Matchup-flex JIT, going first | 16.129% | 44.871% | 60.559% |
| No discard control, going first | 19.854% | 54.925% | 71.071% |
| Strict JIT, going second | 26.76% | 48.546% | 60.126% |
| Matchup-flex JIT, going second | 34.945% | 57.962% | 68.504% |
| No discard control, going second | 39.555% | 66.047% | 77.728% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.382% | 9.704% | 17.096% |
| Strict JIT, full Item lock, first | 2.824% | 7.481% | 14.493% |
| Strict JIT, Rule Box Ability lock, first | 4.388% | 24.113% | 36.339% |
| Strict JIT, combined lock, first | 0.31% | 3.266% | 7.19% |
| Strict JIT, turn-two Item lock, second | 13.519% | 26.826% | 34.086% |
| Strict JIT, full Item lock, second | 10.448% | 22.225% | 29.194% |
| Strict JIT, Rule Box Ability lock, second | 17.278% | 32.378% | 42.544% |
| Strict JIT, combined lock, second | 2.506% | 11.048% | 15.045% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
