# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.445% | 35.392% | 50.64% |
| Matchup-flex JIT, going first | 15.995% | 43.724% | 59.482% |
| No discard control, going first | 19.666% | 54.821% | 71.173% |
| Strict JIT, going second | 27.121% | 48.12% | 58.725% |
| Matchup-flex JIT, going second | 34.082% | 56.61% | 67.494% |
| No discard control, going second | 37.38% | 64.189% | 76.309% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.389% | 9.468% | 16.553% |
| Strict JIT, full Item lock, first | 2.788% | 7.313% | 14.155% |
| Strict JIT, Rule Box Ability lock, first | 4.368% | 23.615% | 35.347% |
| Strict JIT, combined lock, first | 0.327% | 3.249% | 7.163% |
| Strict JIT, turn-two Item lock, second | 13.079% | 25.608% | 32.726% |
| Strict JIT, full Item lock, second | 10.178% | 21.531% | 28.345% |
| Strict JIT, Rule Box Ability lock, second | 17.364% | 31.802% | 40.99% |
| Strict JIT, combined lock, second | 2.517% | 11.058% | 14.926% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
