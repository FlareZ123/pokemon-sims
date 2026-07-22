# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.756% | 37.933% | 55.014% |
| Matchup-flex JIT, going first | 16.142% | 47.41% | 63.246% |
| No discard control, going first | 19.963% | 55.349% | 71.524% |
| Strict JIT, going second | 29.122% | 52.229% | 63.521% |
| Matchup-flex JIT, going second | 36.887% | 60.419% | 70.842% |
| No discard control, going second | 39.949% | 67% | 78.492% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.561% | 10.161% | 17.686% |
| Strict JIT, full Item lock, first | 2.821% | 7.736% | 15.064% |
| Strict JIT, Rule Box Ability lock, first | 4.342% | 25.565% | 38.589% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.27% |
| Strict JIT, turn-two Item lock, second | 14.092% | 27.903% | 35.554% |
| Strict JIT, full Item lock, second | 10.493% | 22.882% | 30.032% |
| Strict JIT, Rule Box Ability lock, second | 18.13% | 34.547% | 44.747% |
| Strict JIT, combined lock, second | 2.352% | 11.342% | 15.418% |
| Strict JIT, Supporter lock, first | 0.002% | 10.236% | 17.118% |
| Strict JIT, Supporter lock, second | 6.118% | 14.508% | 20.918% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
