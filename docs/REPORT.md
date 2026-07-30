# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.246% | 40% | 56.94% |
| Matchup-flex JIT, going first | 16.392% | 48.12% | 64.051% |
| No discard control, going first | 19.985% | 56.04% | 72.375% |
| Strict JIT, going second | 29.912% | 53.746% | 64.973% |
| Matchup-flex JIT, going second | 37.426% | 61.234% | 71.468% |
| No discard control, going second | 39.971% | 67.119% | 78.4% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.551% | 10.285% | 18.044% |
| Strict JIT, full Item lock, first | 2.839% | 7.771% | 15.26% |
| Strict JIT, Rule Box Ability lock, first | 4.36% | 26.622% | 39.998% |
| Strict JIT, combined lock, first | 0.313% | 3.36% | 7.469% |
| Strict JIT, turn-two Item lock, second | 14.221% | 28.11% | 35.873% |
| Strict JIT, full Item lock, second | 10.564% | 23.252% | 30.573% |
| Strict JIT, Rule Box Ability lock, second | 18.077% | 35.339% | 45.878% |
| Strict JIT, combined lock, second | 2.51% | 11.514% | 15.884% |
| Strict JIT, Supporter lock, first | 0.001% | 15.327% | 21.605% |
| Strict JIT, Supporter lock, second | 8.192% | 19.69% | 25.501% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
