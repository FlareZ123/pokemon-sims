# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.158% | 41.807% | 59.222% |
| Matchup-flex JIT, going first | 17.172% | 50.772% | 68.179% |
| No discard control, going first | 19.74% | 56.829% | 73.585% |
| Strict JIT, going second | 29.667% | 55.561% | 67.523% |
| Matchup-flex JIT, going second | 37.321% | 64.185% | 75.793% |
| No discard control, going second | 40.232% | 68.436% | 79.981% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.583% | 10.484% | 18.587% |
| Strict JIT, Rule Box Ability lock, first | 4.524% | 27.794% | 42.757% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.779% | 4.667% | 9.645% |
| Strict JIT, turn-two Item lock, second | 14.203% | 28.493% | 37.272% |
| Strict JIT, Rule Box Ability lock, second | 18.426% | 37.117% | 49.31% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.924% | 14.705% | 20.389% |
| Strict JIT, Supporter lock, first | 0.005% | 16.426% | 23.376% |
| Strict JIT, Supporter lock, second | 7.85% | 20.357% | 26.735% |
| Garbodor + Boost Shake Ability lock, first | 5.604% | 25.133% | 37.799% |
| Garbodor + Boost Shake Ability lock, second | 15.389% | 30.67% | 42.053% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
