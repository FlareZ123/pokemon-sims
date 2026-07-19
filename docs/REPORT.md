# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.017% | 36.055% | 52.772% |
| Matchup-flex JIT, going first | 16.017% | 46.775% | 62.802% |
| No discard control, going first | 19.981% | 55.365% | 71.755% |
| Strict JIT, going second | 27.554% | 49.782% | 61.41% |
| Matchup-flex JIT, going second | 35.688% | 59.227% | 70.068% |
| No discard control, going second | 39.638% | 66.753% | 78.218% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.479% | 10.085% | 17.668% |
| Strict JIT, full Item lock, first | 2.727% | 7.609% | 14.789% |
| Strict JIT, Rule Box Ability lock, first | 4.276% | 25.138% | 37.929% |
| Strict JIT, combined lock, first | 0.313% | 3.271% | 7.237% |
| Strict JIT, turn-two Item lock, second | 13.704% | 27.112% | 34.504% |
| Strict JIT, full Item lock, second | 10.424% | 22.258% | 29.217% |
| Strict JIT, Rule Box Ability lock, second | 17.697% | 33.136% | 43.425% |
| Strict JIT, combined lock, second | 2.518% | 11.082% | 15.115% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
