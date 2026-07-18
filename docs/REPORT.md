# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.782% | 33.51% | 46.342% |
| Matchup-flex JIT, going first | 15.245% | 42.051% | 56.892% |
| No discard control, going first | 18.66% | 52.001% | 67.002% |
| Strict JIT, going second | 25.5% | 46.391% | 56.228% |
| Matchup-flex JIT, going second | 32.251% | 54.337% | 64.904% |
| No discard control, going second | 35.647% | 60.849% | 72.004% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.228% | 9.018% | 14.936% |
| Strict JIT, full Item lock, first | 2.754% | 6.917% | 12.201% |
| Strict JIT, Rule Box Ability lock, first | 4.257% | 22.55% | 33.395% |
| Strict JIT, combined lock, first | 0.276% | 3.043% | 6.499% |
| Strict JIT, turn-two Item lock, second | 11.693% | 23.276% | 30.461% |
| Strict JIT, full Item lock, second | 9.182% | 19.59% | 25.987% |
| Strict JIT, Rule Box Ability lock, second | 16.579% | 30.665% | 39.346% |
| Strict JIT, combined lock, second | 2.317% | 9.534% | 13.366% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
