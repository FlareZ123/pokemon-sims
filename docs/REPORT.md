# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.256% | 30.379% | 42.841% |
| Matchup-flex JIT, going first | 13.128% | 36.03% | 49.349% |
| No discard control, going first | 15.538% | 50.81% | 65.286% |
| Strict JIT, going second | 24.952% | 43.387% | 52.254% |
| Matchup-flex JIT, going second | 31.623% | 50.242% | 58.89% |
| No discard control, going second | 34.552% | 60.235% | 70.971% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 3.791% | 8.019% | 12.388% |
| Strict JIT, full Item lock, first | 2.62% | 6.265% | 10.134% |
| Strict JIT, Rule Box Ability lock, first | 4.053% | 18.505% | 28.107% |
| Strict JIT, combined lock, first | 0.29% | 2.396% | 4.72% |
| Strict JIT, turn-two Item lock, second | 12.464% | 23.612% | 28.968% |
| Strict JIT, full Item lock, second | 9.827% | 19.564% | 24.611% |
| Strict JIT, Rule Box Ability lock, second | 16.169% | 28.063% | 35.241% |
| Strict JIT, combined lock, second | 2.289% | 9.44% | 11.793% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
