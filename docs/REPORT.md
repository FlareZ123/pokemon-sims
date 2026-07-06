# Regidrago VSTAR Setup Report: Trace-Audited Baseline

## Status

This report supersedes the earlier aggregate-only report on `fix-sims`. The engine now exposes deterministic step-by-step traces and the reviewed paths are documented in [`TRACE_AUDIT.md`](TRACE_AUDIT.md). The complete game-mechanic map is in [`RULES_TRACEABILITY.md`](RULES_TRACEABILITY.md).

## Ready-state definition

The simulator counts a ready state only when Regidrago VSTAR is Active, has at least GGF attached, and has a modeled Dragon payload in discard. Strict and matchup-flex JIT require that payload to have entered discard in the ready turn. No-discard-control permits prior-turn payload banking.

## 100,000-trial baseline

Seed: `20260705`.

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 2.192% | 9.928% | 16.813% |
| Matchup-flex JIT, going first | 4.882% | 16.462% | 25.896% |
| No discard control, going first | 7.736% | 27.906% | 39.646% |
| Strict JIT, going second | 15.678% | 25.174% | 30.955% |
| Matchup-flex JIT, going second | 23.461% | 34.994% | 42.348% |
| No discard control, going second | 28.368% | 45.038% | 53.641% |

The raw output is [`../results/simulation_results.csv`](../results/simulation_results.csv). Standard errors are included in that file.

## Lock stress tests

| Scenario | T2 | T3 | T4 |
|---|---:|---:|---:|
| Strict JIT, turn-two Item lock, first | 0.635% | 1.904% | 3.217% |
| Strict JIT, full Item lock, first | 0.563% | 1.664% | 2.808% |
| Strict JIT, Rule Box Ability lock, first | 0.686% | 4.564% | 8.108% |
| Strict JIT, combined lock, first | 0.000% | 0.462% | 0.906% |
| Strict JIT, turn-two Item lock, second | 2.335% | 13.672% | 15.953% |
| Strict JIT, full Item lock, second | 2.104% | 10.122% | 11.825% |
| Strict JIT, Rule Box Ability lock, second | 9.423% | 14.585% | 17.852% |
| Strict JIT, combined lock, second | 0.792% | 4.165% | 4.866% |

## Interpretation

The JIT tax remains large. For example, going first, strict JIT reaches T3 readiness in 9.928% of trials compared with 27.906% for early payload banking. Going second, the corresponding values are 25.174% and 45.038%.

The Rule Box-lock scenario is intentionally modeled as a Path-style suppression of Rule Box Pokémon abilities. Oricorio is not a Rule Box Pokémon and remains active. The model separately handles Forest Seal Stone as an existing Tool VSTAR Power, as documented in the rule map.

## Boundary

The results describe this specific policy engine, not match win rate. Opponent damage, prizes taken, hand disruption, gust, Copycat/Mimikyu, Return Label, Lysandre Prism Star, Stadium wars, and full Expanded legality are outside scope.
