# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.182% | 39.922% | 56.828% |
| Matchup-flex JIT, going first | 16.497% | 48.318% | 64.074% |
| No discard control, going first | 19.956% | 56.021% | 72.357% |
| Strict JIT, going second | 30.058% | 53.968% | 65.248% |
| Matchup-flex JIT, going second | 37.319% | 61.378% | 71.49% |
| No discard control, going second | 39.964% | 67.123% | 78.368% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.551% | 10.285% | 18.044% |
| Strict JIT, full Item lock, first | 2.839% | 7.771% | 15.26% |
| Strict JIT, Rule Box Ability lock, first | 4.41% | 26.572% | 40.162% |
| Strict JIT, combined lock, first | 0.313% | 3.357% | 7.468% |
| Strict JIT, turn-two Item lock, second | 14.204% | 28.109% | 35.87% |
| Strict JIT, full Item lock, second | 10.565% | 23.255% | 30.566% |
| Strict JIT, Rule Box Ability lock, second | 18.226% | 35.522% | 46.064% |
| Strict JIT, combined lock, second | 2.514% | 11.507% | 15.879% |
| Strict JIT, Supporter lock, first | 0.003% | 15.466% | 21.733% |
| Strict JIT, Supporter lock, second | 8.152% | 19.442% | 25.322% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
