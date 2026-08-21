# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.177% | 41.82% | 59.369% |
| Matchup-flex JIT, going first | 17.216% | 50.825% | 68.234% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.856% | 55.696% | 67.607% |
| Matchup-flex JIT, going second | 37.499% | 64.39% | 76.023% |
| No discard control, going second | 40.448% | 68.575% | 80.147% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.523% | 10.518% | 18.591% |
| Strict JIT, Rule Box Ability lock, first | 4.499% | 28.053% | 43.019% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% | 4.667% | 9.645% |
| Strict JIT, turn-two Item lock, second | 14.077% | 28.453% | 37.255% |
| Strict JIT, Rule Box Ability lock, second | 18.54% | 37.31% | 49.713% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.874% | 14.683% | 20.372% |
| Strict JIT, Supporter lock, first | 0.005% | 16.425% | 23.367% |
| Strict JIT, Supporter lock, second | 7.918% | 20.453% | 26.831% |
| Garbodor + Boost Shake Ability lock, first | 5.66% | 25.146% | 37.724% |
| Garbodor + Boost Shake Ability lock, second | 15.438% | 30.684% | 41.874% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
