# Regidrago VSTAR Setup Report: Corrected Setup-Order Baseline

## Status

This report is generated from the executable after removing the unsupported second shuffle between opening Active/Bench placement and Prize placement. The setup procedure source is https://www.pokemon.com/us/pokemon-tcg/rules. The exact generated trace manifest is [`../results/baseline_manifest.json`](../results/baseline_manifest.json), and the raw matrix is [`../results/simulation_results.csv`](../results/simulation_results.csv).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 12.19% | 41.278% | 58.226% |
| Matchup-flex JIT, going first | 16.555% | 49.883% | 65.988% |
| No discard control, going first | 19.958% | 56.02% | 72.356% |
| Strict JIT, going second | 29.812% | 54.713% | 66.296% |
| Matchup-flex JIT, going second | 37.051% | 62.171% | 72.933% |
| No discard control, going second | 39.964% | 67.122% | 78.369% |

## Lock stress tests

Turn-one full Item-lock rows are intentionally omitted and must not be reintroduced as current-paper Expanded matchup scenarios. The official turn procedure prevents the starting player from attacking on the first turn, and Forest of Giant Plants, the historical immediate-evolution enabler for turn-one Vileplume-style locks, is banned in Expanded. Use the turn-two Item-lock rows instead. Combined lock means Rule Box Ability suppression plus Item lock beginning on turn 2. Sources: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/ https://github.com/FlareZ123/pokemon-sims/issues/2247

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 4.625% | 10.455% | 18.344% |
| Strict JIT, Rule Box Ability lock, first | 4.442% | 26.842% | 40.533% |
| Strict JIT, turn-two Item + Rule Box Ability lock, first | 0.814% | 4.665% | 9.585% |
| Strict JIT, turn-two Item lock, second | 14.233% | 28.553% | 37.057% |
| Strict JIT, Rule Box Ability lock, second | 18.503% | 35.942% | 46.816% |
| Strict JIT, turn-two Item + Rule Box Ability lock, second | 3.797% | 14.322% | 19.877% |
| Strict JIT, Supporter lock, first | 0.002% | 14.719% | 21.186% |
| Strict JIT, Supporter lock, second | 8.046% | 19.303% | 25.257% |

## Interpretation boundary

These percentages estimate setup readiness for this policy engine. They are not match-win rates. Opponent damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing, and full Expanded legality remain outside this goldfish model.
