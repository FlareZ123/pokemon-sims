# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.11% | 41.536% | 58.934% |
| Matchup-flex JIT, going first | 17.146% | 50.68% | 67.4% |
| No discard control, going first | 19.764% | 56.888% | 73.672% |
| Strict JIT, going second | 29.844% | 55.061% | 67.129% |
| Matchup-flex JIT, going second | 37.338% | 62.956% | 74.328% |
| No discard control, going second | 40.003% | 68.304% | 79.94% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.562% | 10.472% | 18.39% |
| Strict JIT, Rule Box Ability lock, first | 4.541% | 27.149% | 41.655% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.782% | 4.639% | 9.64% |
| Strict JIT, turn-two Item lock, second | 14.153% | 28.429% | 36.881% |
| Strict JIT, Rule Box Ability lock, second | 18.4% | 36.436% | 48.333% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.8% | 14.486% | 20.005% |
| Strict JIT, Supporter lock, first | 0.001% | 14.758% | 21.471% |
| Strict JIT, Supporter lock, second | 7.968% | 19.149% | 25.41% |
| Garbodor + Boost Shake Ability lock, first | 5.816% | 27.301% | 41.081% |
| Garbodor + Boost Shake Ability lock, second | 17.318% | 34.384% | 46.242% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
