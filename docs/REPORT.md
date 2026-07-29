# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.961% | 38.632% | 55.388% |
| Matchup-flex JIT, going first | 16.15% | 47.417% | 63.424% |
| No discard control, going first | 20.134% | 56% | 72.128% |
| Strict JIT, going second | 29.661% | 52.676% | 63.961% |
| Matchup-flex JIT, going second | 37.463% | 61.107% | 71.304% |
| No discard control, going second | 39.83% | 66.914% | 77.999% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.54% | 10.287% | 17.767% |
| Strict JIT, full Item lock, first | 2.842% | 7.664% | 14.878% |
| Strict JIT, Rule Box Ability lock, first | 4.4% | 25.783% | 38.862% |
| Strict JIT, combined lock, first | 0.297% | 3.346% | 7.345% |
| Strict JIT, turn-two Item lock, second | 13.945% | 27.792% | 35.487% |
| Strict JIT, full Item lock, second | 10.545% | 22.756% | 29.926% |
| Strict JIT, Rule Box Ability lock, second | 18.328% | 34.953% | 45.183% |
| Strict JIT, combined lock, second | 2.369% | 11.349% | 15.404% |
| Strict JIT, Supporter lock, first | 0.003% | 15.383% | 21.787% |
| Strict JIT, Supporter lock, second | 8.131% | 19.386% | 25.491% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
