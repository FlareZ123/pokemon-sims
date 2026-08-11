# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.139% | 41.598% | 59.03% |
| Matchup-flex JIT, going first | 17.225% | 50.523% | 67.428% |
| No discard control, going first | 19.747% | 56.986% | 73.75% |
| Strict JIT, going second | 29.631% | 54.957% | 67.02% |
| Matchup-flex JIT, going second | 37.392% | 62.879% | 74.419% |
| No discard control, going second | 39.935% | 68.33% | 79.952% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.627% | 10.482% | 18.429% |
| Strict JIT, Rule Box Ability lock, first | 4.595% | 27.146% | 41.52% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.8% | 4.683% | 9.579% |
| Strict JIT, turn-two Item lock, second | 14.324% | 28.591% | 37.151% |
| Strict JIT, Rule Box Ability lock, second | 18.431% | 36.389% | 48.099% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.799% | 14.325% | 19.914% |
| Strict JIT, Supporter lock, first | 0.001% | 14.769% | 21.486% |
| Strict JIT, Supporter lock, second | 7.971% | 19.169% | 25.417% |
| Garbodor + Boost Shake Ability lock, first | 5.737% | 27.243% | 41.038% |
| Garbodor + Boost Shake Ability lock, second | 17.193% | 34.318% | 46.141% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
