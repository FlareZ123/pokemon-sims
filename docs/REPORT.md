# Regidrago VSTAR Live Expanded Setup Report

## Status: superseded and regenerated on `fix-sims`

The original report was produced before the simulator audit documented in [`SIMULATOR_AUDIT.md`](SIMULATOR_AUDIT.md). Its percentages must not be compared with the corrected result artifacts. The audit fixed entry-Ability timing, Oricorio's non-Rule-Box status, Earthen Vessel and Crispin Energy rules, hidden-information leaks, turn-one no-control payload banking, Tate & Liza switching, and connector-domination policy flaws.

This document reports the regenerated 100,000-trial baseline from the audited implementation. The model remains a single-player, turn-4 setup policy simulator. It does not model opponent attacks, prize-taking, damage, gust, hand disruption, Return Label, Lysandre Prism Star, Surprise Box, Girafarig, Mimikyu Copycat, or complete two-player game trees.

## Setup-ready predicate

A trial is ready when all conditions are true:

1. Regidrago VSTAR is Active.
2. It has at least two Grass Energy and one Fire Energy attached.
3. An A- or S-tier payload is in the discard pile.
4. In strict and matchup-flex JIT profiles, the payload entered discard during that same turn.
5. The player has reached turn 2 or later.

The eligible payloads are Dragapult ex, Mega Dragonite ex, Dialga-GX, and Hisuian Goodra VSTAR.

## Corrected baseline probability

| Scenario | By T2 | By T3 | By T4 | T3 Monte Carlo SE |
|---|---:|---:|---:|---:|
| Strict JIT, going first | 6.592% | 15.268% | 22.126% | 0.114 pp |
| Matchup-flex JIT, going first | 6.417% | 19.144% | 29.043% | 0.124 pp |
| No discard control, going first | 9.573% | 28.407% | 41.902% | 0.143 pp |
| Strict JIT, going second | 14.800% | 22.110% | 27.332% | 0.131 pp |
| Matchup-flex JIT, going second | 18.842% | 27.411% | 34.656% | 0.141 pp |
| No discard control, going second | 17.507% | 33.644% | 44.219% | 0.149 pp |

### Lock stress tests

| Scenario | By T2 | By T3 | By T4 |
|---|---:|---:|---:|
| Strict JIT, turn-2 Item lock, first | 0.000% | 1.628% | 2.909% |
| Strict JIT, full Item lock, first | 0.000% | 1.154% | 2.195% |
| Strict JIT, Rule Box Ability lock, first | 2.137% | 6.965% | 10.369% |
| Strict JIT, combined lock, first | 0.000% | 0.673% | 1.180% |
| Strict JIT, turn-2 Item lock, second | 0.434% | 10.415% | 12.260% |
| Strict JIT, full Item lock, second | 0.362% | 7.632% | 9.003% |
| Strict JIT, Rule Box Ability lock, second | 7.586% | 11.523% | 14.579% |
| Strict JIT, combined lock, second | 0.264% | 3.333% | 3.982% |

## Corrected JIT tax

| Seat | Deadline | Strict JIT | Matchup-flex JIT | No discard control | Strict tax versus no control |
|---|---|---:|---:|---:|---:|
| Going first | T2 | 6.592% | 6.417% | 9.573% | 2.981 pp |
| Going first | T3 | 15.268% | 19.144% | 28.407% | 13.139 pp |
| Going first | T4 | 22.126% | 29.043% | 41.902% | 19.776 pp |
| Going second | T2 | 14.800% | 18.842% | 17.507% | 2.707 pp |
| Going second | T3 | 22.110% | 27.411% | 33.644% | 11.534 pp |
| Going second | T4 | 27.332% | 34.656% | 44.219% | 16.887 pp |

The strict JIT comparison is now valid against a non-control profile that permits legal turn-one payload banking. Matchup-flex JIT can underperform strict JIT at a particular deadline because the policy may use matchup cards as discard fuel and alter later connector availability. It should be interpreted as a distinct policy profile rather than a guaranteed monotonic upper bound.

## Corrected swap screen

Each row uses 25,000 matched-seed trials. Values below show turn-3 change versus the matching corrected baseline.

| Variant | Strict first | Strict second | Flex first | Turn-2 Item lock second |
|---|---:|---:|---:|---:|
| Mawile-GX → Crispin | +1.788 pp | +3.104 pp | +2.128 pp | +0.076 pp |
| Mawile-GX → Steven's Resolve | +0.996 pp | +3.456 pp | +0.960 pp | +2.800 pp |
| Mawile-GX → Professor Burnet | +1.584 pp | +1.772 pp | +2.332 pp | +1.888 pp |
| Mawile-GX → Evolution Incense | +1.808 pp | +1.908 pp | +2.692 pp | +0.388 pp |
| Mawile-GX + Tate & Liza → Quick Ball + Crispin | +2.136 pp | +3.464 pp | +3.932 pp | +0.516 pp |

These are setup-speed screens. They do not price Mawile-GX's anti-Copycat value, Guzma's gust and switch value, Field Blower's Tool/Stadium interaction, or other matchup-driven value after the first Apex-ready state.

## Reproduction

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
build\\Release\\regidrago_sim.exe --trials 100000 --variant-trials 25000 --seed 20260705 --out results\\simulation_results.csv --variants-out results\\variant_results.csv
```

The executable name is commonly `build/regidrago_sim` on single-configuration generators.
