# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.645% | 33.07% | 46.252% |
| Matchup-flex JIT, going first | 13.626% | 40.451% | 55.679% |
| No discard control, going first | 15.976% | 51.748% | 66.864% |
| Strict JIT, going second | 25.395% | 46.061% | 55.833% |
| Matchup-flex JIT, going second | 31.661% | 54.07% | 64.71% |
| No discard control, going second | 35.452% | 60.74% | 71.903% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 3.805% | 8.647% | 13.838% |
| Strict JIT, full Item lock, first | 2.638% | 6.762% | 11.514% |
| Strict JIT, Rule Box Ability lock, first | 4.124% | 22.002% | 32.696% |
| Strict JIT, combined lock, first | 0.307% | 3.007% | 6.07% |
| Strict JIT, turn-two Item lock, second | 11.439% | 22.72% | 29.573% |
| Strict JIT, full Item lock, second | 9.1% | 19.195% | 25.186% |
| Strict JIT, Rule Box Ability lock, second | 16.438% | 30.545% | 38.913% |
| Strict JIT, combined lock, second | 2.198% | 9.147% | 12.714% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
