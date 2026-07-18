# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.51% | 35.203% | 50.271% |
| Matchup-flex JIT, going first | 15.793% | 43.391% | 59.021% |
| No discard control, going first | 19.468% | 53.944% | 70.128% |
| Strict JIT, going second | 26.871% | 47.723% | 58.291% |
| Matchup-flex JIT, going second | 33.693% | 56.138% | 66.69% |
| No discard control, going second | 37.323% | 63.458% | 75.558% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.492% | 9.636% | 16.586% |
| Strict JIT, full Item lock, first | 2.88% | 7.424% | 14.007% |
| Strict JIT, Rule Box Ability lock, first | 4.45% | 23.563% | 35.462% |
| Strict JIT, combined lock, first | 0.292% | 3.237% | 7.065% |
| Strict JIT, turn-two Item lock, second | 13.071% | 25.664% | 32.573% |
| Strict JIT, full Item lock, second | 10.204% | 21.43% | 28.022% |
| Strict JIT, Rule Box Ability lock, second | 17.314% | 31.712% | 40.936% |
| Strict JIT, combined lock, second | 2.42% | 10.812% | 14.411% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
