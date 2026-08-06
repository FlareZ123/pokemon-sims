# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `424242`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.422% | 40.152% | 57.178% |
| Matchup-flex JIT, going first | 16.522% | 48.357% | 64.386% |
| No discard control, going first | 20.286% | 56.177% | 72.428% |
| Strict JIT, going second | 29.788% | 53.67% | 65.353% |
| Matchup-flex JIT, going second | 37.519% | 61.532% | 72.067% |
| No discard control, going second | 40.27% | 67.321% | 78.659% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.408% | 10.257% | 18.066% |
| Strict JIT, full Item lock, first | 2.904% | 8.005% | 15.378% |
| Strict JIT, Rule Box Ability lock, first | 4.382% | 26.449% | 40.209% |
| Strict JIT, combined lock, first | 0.344% | 3.334% | 7.372% |
| Strict JIT, turn-two Item lock, second | 14.053% | 28.263% | 36.699% |
| Strict JIT, full Item lock, second | 10.716% | 23.458% | 31.353% |
| Strict JIT, Rule Box Ability lock, second | 18.145% | 35.41% | 46.101% |
| Strict JIT, combined lock, second | 2.506% | 11.556% | 16.006% |
| Strict JIT, Supporter lock, first | 0.004% | 15.315% | 21.648% |
| Strict JIT, Supporter lock, second | 8.099% | 19.407% | 25.11% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
