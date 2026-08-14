# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.192% | 41.731% | 59.192% |
| Matchup-flex JIT, going first | 17.3% | 50.921% | 68.213% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.665% | 55.502% | 67.514% |
| Matchup-flex JIT, going second | 37.233% | 64.147% | 75.8% |
| No discard control, going second | 40.232% | 68.436% | 79.981% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.493% | 10.34% | 18.468% |
| Strict JIT, Rule Box Ability lock, first | 4.518% | 27.734% | 42.674% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.765% | 4.61% | 9.628% |
| Strict JIT, turn-two Item lock, second | 14.127% | 28.377% | 37.109% |
| Strict JIT, Rule Box Ability lock, second | 18.465% | 37.134% | 49.355% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.9% | 14.51% | 20.154% |
| Strict JIT, Supporter lock, first | 0.005% | 16.426% | 23.376% |
| Strict JIT, Supporter lock, second | 7.85% | 20.357% | 26.735% |
| Garbodor + Boost Shake Ability lock, first | 5.659% | 26.893% | 40.976% |
| Garbodor + Boost Shake Ability lock, second | 17.458% | 34.735% | 46.427% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
