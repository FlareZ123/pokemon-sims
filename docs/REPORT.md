# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.907% | 38.506% | 55.425% |
| Matchup-flex JIT, going first | 16.308% | 47.647% | 63.409% |
| No discard control, going first | 20.134% | 56% | 72.128% |
| Strict JIT, going second | 29.303% | 52.635% | 63.878% |
| Matchup-flex JIT, going second | 37.302% | 61.097% | 71.345% |
| No discard control, going second | 39.83% | 66.914% | 77.999% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.574% | 10.157% | 17.686% |
| Strict JIT, full Item lock, first | 2.817% | 7.746% | 15.059% |
| Strict JIT, Rule Box Ability lock, first | 4.383% | 26.02% | 39.115% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.268% |
| Strict JIT, turn-two Item lock, second | 14.068% | 27.943% | 35.549% |
| Strict JIT, full Item lock, second | 10.558% | 22.93% | 30.109% |
| Strict JIT, Rule Box Ability lock, second | 18.012% | 34.65% | 44.938% |
| Strict JIT, combined lock, second | 2.365% | 11.404% | 15.519% |
| Strict JIT, Supporter lock, first | 0.001% | 15.327% | 21.605% |
| Strict JIT, Supporter lock, second | 8.192% | 19.69% | 25.501% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
