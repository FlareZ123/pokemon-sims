# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.729% | 38.089% | 55.203% |
| Matchup-flex JIT, going first | 16.266% | 47.255% | 63.041% |
| No discard control, going first | 19.963% | 55.349% | 71.524% |
| Strict JIT, going second | 29.197% | 52.357% | 63.685% |
| Matchup-flex JIT, going second | 37.051% | 60.641% | 70.91% |
| No discard control, going second | 39.949% | 67% | 78.492% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.583% | 10.183% | 17.696% |
| Strict JIT, full Item lock, first | 2.821% | 7.736% | 15.064% |
| Strict JIT, Rule Box Ability lock, first | 4.365% | 25.663% | 38.835% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.27% |
| Strict JIT, turn-two Item lock, second | 14.201% | 28.029% | 35.672% |
| Strict JIT, full Item lock, second | 10.493% | 22.882% | 30.032% |
| Strict JIT, Rule Box Ability lock, second | 18.164% | 34.62% | 44.952% |
| Strict JIT, combined lock, second | 2.352% | 11.342% | 15.418% |
| Strict JIT, Supporter lock, first | 0.002% | 10.065% | 16.99% |
| Strict JIT, Supporter lock, second | 6.112% | 14.528% | 20.991% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
