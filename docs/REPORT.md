# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 11.922% | 38.493% | 55.377% |
| Matchup-flex JIT, going first | 16.274% | 47.59% | 63.396% |
| No discard control, going first | 20.134% | 56% | 72.128% |
| Strict JIT, going second | 29.488% | 52.611% | 63.817% |
| Matchup-flex JIT, going second | 37.223% | 60.744% | 71.022% |
| No discard control, going second | 39.83% | 66.914% | 77.999% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.596% | 10.206% | 17.699% |
| Strict JIT, full Item lock, first | 2.823% | 7.749% | 15.061% |
| Strict JIT, Rule Box Ability lock, first | 4.468% | 25.993% | 39.067% |
| Strict JIT, combined lock, first | 0.293% | 3.279% | 7.268% |
| Strict JIT, turn-two Item lock, second | 14.085% | 27.964% | 35.542% |
| Strict JIT, full Item lock, second | 10.556% | 22.936% | 30.105% |
| Strict JIT, Rule Box Ability lock, second | 18.125% | 34.712% | 44.849% |
| Strict JIT, combined lock, second | 2.373% | 11.415% | 15.519% |
| Strict JIT, Supporter lock, first | 0.001% | 15.369% | 21.653% |
| Strict JIT, Supporter lock, second | 8.208% | 19.692% | 25.509% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
