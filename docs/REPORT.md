# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.234% | 41.859% | 59.39% |
| Matchup-flex JIT, going first | 17.152% | 50.658% | 68.089% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.793% | 55.668% | 67.498% |
| Matchup-flex JIT, going second | 37.424% | 64.317% | 75.962% |
| No discard control, going second | 40.448% | 68.575% | 80.147% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.523% | 10.518% | 18.591% |
| Strict JIT, Rule Box Ability lock, first | 4.545% | 28.165% | 43.128% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% | 4.667% | 9.645% |
| Strict JIT, turn-two Item lock, second | 14.077% | 28.453% | 37.255% |
| Strict JIT, Rule Box Ability lock, second | 18.419% | 37.234% | 49.574% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.874% | 14.683% | 20.372% |
| Strict JIT, Supporter lock, first | 0.005% | 16.426% | 23.376% |
| Strict JIT, Supporter lock, second | 7.852% | 20.358% | 26.736% |
| Garbodor + Boost Shake Ability lock, first | 5.602% | 25.112% | 37.757% |
| Garbodor + Boost Shake Ability lock, second | 15.436% | 30.671% | 41.931% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
