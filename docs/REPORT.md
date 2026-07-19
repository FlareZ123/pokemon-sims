# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 10.993% | 35.449% | 51.256% |
| Matchup-flex JIT, going first | 16.141% | 44.742% | 60.437% |
| No discard control, going first | 19.751% | 54.768% | 71.081% |
| Strict JIT, going second | 26.864% | 48.005% | 59.252% |
| Matchup-flex JIT, going second | 35.298% | 57.464% | 68.21% |
| No discard control, going second | 38.677% | 64.412% | 76.547% |

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.379% | 9.574% | 16.915% |
| Strict JIT, full Item lock, first | 2.818% | 7.359% | 14.358% |
| Strict JIT, Rule Box Ability lock, first | 4.438% | 24.26% | 36.626% |
| Strict JIT, combined lock, first | 0.309% | 3.26% | 7.194% |
| Strict JIT, turn-two Item lock, second | 13.594% | 26.61% | 33.913% |
| Strict JIT, full Item lock, second | 10.399% | 22.117% | 29.074% |
| Strict JIT, Rule Box Ability lock, second | 17.265% | 32.354% | 42.498% |
| Strict JIT, combined lock, second | 2.519% | 11.053% | 15.028% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, opponent-caused Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model. A card-mandated self-Knock-Out may still be executed inside an isolated supported setup route, such as Forretress ex’s Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2.
