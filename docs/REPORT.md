# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.188% | 36.867% | 53.454% |
| Matchup-flex JIT, going first | 16.307% | 46.706% | 62.908% |
| No discard control, going first | 20.014% | 55.449% | 71.556% |
| Strict JIT, going second | 28.121% | 50.08% | 61.455% |
| Matchup-flex JIT, going second | 36.179% | 59.628% | 70.393% |
| No discard control, going second | 39.444% | 66.43% | 78.146% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.457% | 10.153% | 17.708% |
| Strict JIT, full Item lock, first | 2.76% | 7.673% | 14.818% |
| Strict JIT, Rule Box Ability lock, first | 4.454% | 25.068% | 37.84% |
| Strict JIT, combined lock, first | 0.321% | 3.352% | 7.3% |
| Strict JIT, turn-two Item lock, second | 13.988% | 27.25% | 34.638% |
| Strict JIT, full Item lock, second | 10.574% | 22.459% | 29.239% |
| Strict JIT, Rule Box Ability lock, second | 17.734% | 33.391% | 43.519% |
| Strict JIT, combined lock, second | 2.428% | 10.934% | 14.905% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
