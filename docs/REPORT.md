# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.175% | 37.501% | 54.241% |
| Matchup-flex JIT, going first | 16.139% | 46.917% | 63.046% |
| No discard control, going first | 19.831% | 55.336% | 71.514% |
| Strict JIT, going second | 28.181% | 51.149% | 62.71% |
| Matchup-flex JIT, going second | 36.505% | 60.466% | 71.021% |
| No discard control, going second | 39.769% | 66.783% | 78.332% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.645% | 10.311% | 17.772% |
| Strict JIT, full Item lock, first | 2.775% | 7.502% | 14.744% |
| Strict JIT, Rule Box Ability lock, first | 4.383% | 25.646% | 38.695% |
| Strict JIT, combined lock, first | 0.286% | 3.259% | 7.171% |
| Strict JIT, turn-two Item lock, second | 13.84% | 27.351% | 34.733% |
| Strict JIT, full Item lock, second | 10.678% | 22.948% | 29.878% |
| Strict JIT, Rule Box Ability lock, second | 17.943% | 34.365% | 44.58% |
| Strict JIT, combined lock, second | 2.39% | 11.417% | 15.485% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
