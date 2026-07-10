# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 5.904% | 19.796% | 30.809% |
| Matchup-flex JIT, going first | 8.85% | 26.668% | 40.112% |
| No discard control, going first | 10.115% | 38.354% | 53.484% |
| Strict JIT, going second | 23.14% | 38.832% | 47.986% |
| Matchup-flex JIT, going second | 31.099% | 48.514% | 58.097% |
| No discard control, going second | 34.222% | 58.047% | 68.565% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 2.688% | 5.883% | 9.318% |
| Strict JIT, full Item lock, first | 2.044% | 4.802% | 7.805% |
| Strict JIT, Rule Box Ability lock, first | 1.452% | 9.203% | 15.32% |
| Strict JIT, combined lock, first | 0% | 0.991% | 2.331% |
| Strict JIT, turn-two Item lock, second | 11.822% | 23.986% | 28.378% |
| Strict JIT, full Item lock, second | 9.457% | 20.135% | 24.19% |
| Strict JIT, Rule Box Ability lock, second | 13.518% | 21.463% | 26.474% |
| Strict JIT, combined lock, second | 1.197% | 7.652% | 9.189% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
