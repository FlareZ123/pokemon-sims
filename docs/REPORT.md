# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.008% | 35.766% | 52.476% |
| Matchup-flex JIT, going first | 16.113% | 45.885% | 62.131% |
| No discard control, going first | 19.991% | 55.416% | 71.745% |
| Strict JIT, going second | 27.158% | 48.983% | 60.808% |
| Matchup-flex JIT, going second | 35.127% | 58.591% | 69.81% |
| No discard control, going second | 39.494% | 66.728% | 78.237% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.573% | 10.165% | 17.71% |
| Strict JIT, full Item lock, first | 2.763% | 7.547% | 14.826% |
| Strict JIT, Rule Box Ability lock, first | 4.228% | 24.522% | 37.1% |
| Strict JIT, combined lock, first | 0.314% | 3.262% | 7.198% |
| Strict JIT, turn-two Item lock, second | 13.509% | 26.938% | 34.309% |
| Strict JIT, full Item lock, second | 10.495% | 22.294% | 29.311% |
| Strict JIT, Rule Box Ability lock, second | 17.325% | 32.569% | 43.01% |
| Strict JIT, combined lock, second | 2.517% | 11.073% | 15.098% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
