# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.27% | 41.776% | 59.148% |
| Matchup-flex JIT, going first | 16.992% | 50.082% | 67.676% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.834% | 55.357% | 67.331% |
| Matchup-flex JIT, going second | 37.015% | 63.918% | 75.613% |
| No discard control, going second | 40.232% | 68.436% | 79.981% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.523% | 10.585% | 18.743% |
| Strict JIT, Rule Box Ability lock, first | 4.557% | 27.773% | 42.709% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.785% | 4.579% | 9.615% |
| Strict JIT, turn-two Item lock, second | 14.172% | 28.556% | 37.311% |
| Strict JIT, Rule Box Ability lock, second | 18.468% | 37.202% | 49.43% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.788% | 14.423% | 20.104% |
| Strict JIT, Supporter lock, first | 0.001% | 15.007% | 22.007% |
| Strict JIT, Supporter lock, second | 8.005% | 19.39% | 25.846% |
| Garbodor + Boost Shake Ability lock, first | 5.561% | 27.145% | 41.079% |
| Garbodor + Boost Shake Ability lock, second | 17.249% | 34.694% | 46.344% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
