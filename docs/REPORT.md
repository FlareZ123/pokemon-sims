# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.14% | 36.558% | 53.248% |
| Matchup-flex JIT, going first | 16.183% | 46.623% | 62.712% |
| No discard control, going first | 19.93% | 55.332% | 71.457% |
| Strict JIT, going second | 28.282% | 50.312% | 61.746% |
| Matchup-flex JIT, going second | 36.068% | 59.572% | 70.304% |
| No discard control, going second | 39.564% | 66.469% | 78.063% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.449% | 10.168% | 17.695% |
| Strict JIT, full Item lock, first | 2.758% | 7.679% | 14.815% |
| Strict JIT, Rule Box Ability lock, first | 4.354% | 25.247% | 38.354% |
| Strict JIT, combined lock, first | 0.323% | 3.354% | 7.3% |
| Strict JIT, turn-two Item lock, second | 13.858% | 27.198% | 34.605% |
| Strict JIT, full Item lock, second | 10.577% | 22.435% | 29.22% |
| Strict JIT, Rule Box Ability lock, second | 17.752% | 33.348% | 43.771% |
| Strict JIT, combined lock, second | 2.427% | 10.932% | 14.915% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
