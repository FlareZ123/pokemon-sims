# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.08% | 41.196% | 58.185% |
| Matchup-flex JIT, going first | 16.943% | 49.967% | 65.891% |
| No discard control, going first | 19.971% | 56.026% | 72.341% |
| Strict JIT, going second | 29.598% | 54.38% | 66.12% |
| Matchup-flex JIT, going second | 37.471% | 62.401% | 72.985% |
| No discard control, going second | 39.971% | 67.161% | 78.402% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.617% | 10.41% | 18.285% |
| Strict JIT, Rule Box Ability lock, first | 4.55% | 26.996% | 40.657% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.801% | 4.633% | 9.537% |
| Strict JIT, turn-two Item lock, second | 14.203% | 28.479% | 37.045% |
| Strict JIT, Rule Box Ability lock, second | 18.472% | 35.949% | 46.933% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.767% | 14.289% | 19.883% |
| Strict JIT, Supporter lock, first | 0.002% | 14.719% | 21.186% |
| Strict JIT, Supporter lock, second | 8.046% | 19.303% | 25.257% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
