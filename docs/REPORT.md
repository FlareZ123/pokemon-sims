# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.295% | 41.901% | 59.454% |
| Matchup-flex JIT, going first | 17.128% | 50.803% | 68.3% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.848% | 55.822% | 67.55% |
| Matchup-flex JIT, going second | 37.492% | 64.369% | 75.933% |
| No discard control, going second | 40.448% | 68.575% | 80.147% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.523% | 10.518% | 18.591% |
| Strict JIT, Rule Box Ability lock, first | 4.481% | 28.031% | 42.952% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% | 4.667% | 9.645% |
| Strict JIT, turn-two Item lock, second | 14.077% | 28.453% | 37.255% |
| Strict JIT, Rule Box Ability lock, second | 18.55% | 37.33% | 49.729% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.874% | 14.683% | 20.372% |
| Strict JIT, Supporter lock, first | 0.005% | 16.416% | 23.386% |
| Strict JIT, Supporter lock, second | 7.919% | 20.461% | 26.836% |
| Garbodor + Boost Shake Ability lock, first | 5.667% | 25.16% | 37.742% |
| Garbodor + Boost Shake Ability lock, second | 15.421% | 30.684% | 41.895% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
