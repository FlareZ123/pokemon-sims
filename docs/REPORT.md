# Regidrago VSTAR Live Expanded Setup Report

## Executive finding

This project modeled the supplied 60-card list as a goldfish setup engine through the player’s turn 4. A setup-ready state means an **Active Regidrago VSTAR** with **two Grass and one Fire attached**, together with at least one A-tier or S-tier Dragon attack payload available in discard. In strict and matchup-flex JIT scenarios, the payload also must have entered discard during the present turn.

The main speed limiter is the strict JIT payload requirement. Going first, strict JIT reaches readiness by turn 3 in **7.118%** of trials, compared with **18.245%** when payloads may be discarded early. Going second, those figures are **13.529%** and **29.263%**. Matchup-aware DCI recovers part of that gap, though it leaves a large remaining JIT tax.

The strongest screened pure-speed one-card swap was **Mawile-GX → third Crispin**. Its strict-JIT turn-3 gain was **+2.316 percentage points** going first and **+2.264 points** going second. This recommendation is restricted to matchups where Mawile’s anti-Copycat role has low expected value. Removing Mawile indiscriminately would erase a discrete answer that this goldfish simulator cannot price correctly.

## 1. Inputs, rule register, and evidence trail

- `data/decklist.json` holds the exact 19 Pokémon, 32 Trainers, and 9 Basic Energy from the supplied list.
- `data/card_audit.json` is generated from the supplied card corpus. It verifies each selected print’s card ID, type, stage, and text.
- `docs/RULES_AND_INTERACTIONS.md` records the game-sequence rules and interaction rulings used by the engine.
- `SIM-PLAN.md` records the state-space design, revisions, edge-case decisions, and modeling boundary.

The important audited corrections are: Brilliant Blender discards selected cards from the deck; Mysterious Treasure discards one hand card and finds only Psychic or Dragon Pokémon; Dipplin is Stage 1; the Dragon Dialga-GX print is required; Forest Seal Stone can be used through Path to the Peak while Item lock remains absent; and Latias ex only makes Basic Pokémon’s retreat cost zero.

The card corpus can verify card text and paper-format metadata. It cannot prove dynamic Pokémon TCG Live client availability. The supplied Live list is treated as the roster assumption for the analysis.

## 2. Simulator design and coverage

Each trial performs the following sequence:

1. Construct the exact 60-card deck, shuffle, draw seven, and resolve mulligans.
2. Choose an opening Active, make legal opening Bench placements, then take six Prize cards.
3. Draw at the start of every own turn.
4. Enforce one Supporter, one manual Energy attachment, five Bench slots, and both evolution gates.
5. Enforce actual costs for Mysterious Treasure, Quick Ball, Earthen Vessel, Ultra Ball, and their current DCI candidates.
6. Resolve relevant search, bench-trigger, recovery, VSTAR-Power, and discard effects.
7. Evaluate readiness after every turn through turn 4.

The engine tracks exact deck, hand, Prize, discard, `discarded_this_turn`, Active, Bench, Energy, Tool, Supporter, manual-attachment, VSTAR-Power, and turn-end state. Searches select legal cards from the remaining deck. Policy decisions cannot inspect the future draw order.

This is a single-player setup model. It does not simulate opposing attacks, Knock Outs, damage, gust, prize-taking, hand disruption, Return Label, Lysandre Prism Star, Surprise Box, Girafarig, or full match win rates. Requested locks are modeled as explicit external constraints.

## 3. Baseline setup probability

Each baseline row uses 100,000 trials with fixed seed `20260704`. The standard error measures Monte Carlo sampling variation only.

### JIT and discard-control conditions

| Scenario | By T2 | By T3 | By T4 | T3 Monte Carlo SE |
|---|---:|---:|---:|---:|
| Strict JIT, first | 0.659% | 7.118% | 15.395% | 0.081 pp |
| Matchup-flex JIT, first | 1.096% | 10.844% | 21.872% | 0.098 pp |
| No discard-control, first | 1.466% | 18.245% | 35.579% | 0.122 pp |
| Strict JIT, second | 5.230% | 13.529% | 20.562% | 0.108 pp |
| Matchup-flex JIT, second | 8.942% | 19.237% | 28.661% | 0.125 pp |
| No discard-control, second | 9.915% | 29.263% | 42.507% | 0.144 pp |

### Lock stress tests

| Scenario | By T2 | By T3 | By T4 | T3 Monte Carlo SE |
|---|---:|---:|---:|---:|
| Strict JIT, turn-2 Item lock, first | 0.000% | 0.929% | 2.097% | 0.030 pp |
| Strict JIT, full Item lock, first | 0.000% | 0.811% | 1.858% | 0.028 pp |
| Strict JIT, Rule Box Ability lock, first | 0.512% | 4.107% | 7.271% | 0.063 pp |
| Strict JIT, combined lock, first | 0.000% | 0.579% | 1.096% | 0.024 pp |
| Strict JIT, turn-2 Item lock, second | 0.000% | 2.407% | 4.456% | 0.048 pp |
| Strict JIT, full Item lock, second | 0.000% | 1.990% | 3.380% | 0.044 pp |
| Strict JIT, Rule Box Ability lock, second | 4.091% | 8.476% | 11.176% | 0.088 pp |
| Strict JIT, combined lock, second | 0.000% | 1.225% | 1.897% | 0.035 pp |

### Readout

- Opening Regidrago V occurred in roughly **54%** of trials. Mean mulligans were roughly **0.35** per game.
- Going second provides a material tempo increase because turn-one Supporters become available. The strict-JIT turn-2 result rises from **0.659%** going first to **5.230%** going second.
- Full Item lock is crushing because it removes Blender, Mysterious Treasure, Quick Ball, Earthen Vessel, Forest Seal Stone, and Heavy Ball. A turn-2 Item lock still allows a normal first turn, which explains its modestly better result.
- Rule Box Ability lock suppresses Tapu Lele-GX, Oricorio, and Latias ex. Forest Seal Stone remains a live route in the model when Items remain legal, so this condition is less severe than combined lock.

## 4. JIT tax and matchup-dependent DCI

The following comparison isolates the price of preserving attacks until the same attack turn. “Flex credit” is the benefit from turning visibly low-value matchup cards into legal discard fuel while still enforcing same-turn payload placement. “Full JIT tax” compares strict JIT against early payload banking.

| Seat | Deadline | Strict JIT | Matchup-flex JIT | No discard-control | Flex credit | Full JIT tax | Strict loss versus non-control |
|---|---|---:|---:|---:|---:|---:|---:|
| first | T2 | 0.659% | 1.096% | 1.466% | +0.437 pp | +0.807 pp | 55.0% |
| first | T3 | 7.118% | 10.844% | 18.245% | +3.726 pp | +11.127 pp | 61.0% |
| first | T4 | 15.395% | 21.872% | 35.579% | +6.477 pp | +20.184 pp | 56.7% |
| second | T2 | 5.230% | 8.942% | 9.915% | +3.712 pp | +4.685 pp | 47.3% |
| second | T3 | 13.529% | 19.237% | 29.263% | +5.708 pp | +15.734 pp | 53.8% |
| second | T4 | 20.562% | 28.661% | 42.507% | +8.099 pp | +21.945 pp | 51.6% |

The full strict-JIT cost is largest by turn 4: **20.184 points** going first and **21.945 points** going second. Strict JIT retains only **43.3%** of the non-control turn-4 readiness going first and **48.4%** going second.

This is the core AMR conclusion. A connector graph that counts Mysterious Treasure, Quick Ball, and Earthen Vessel as universally available would overstate the deck sharply. Each connector can exist in hand while having no legal high-DCI payment. Blender has unusually strong JIT AMR because it does not spend a protected hand card to discard the selected Dragon.

## 5. Swap screen

Every swap row uses 25,000 matched-seed trials in each listed scenario. Values are turn-3 percentage-point deltas against the exact baseline policy. Treat values under about 0.5 percentage points as screening noise unless a later high-sample confirmation agrees.

| Variant | Strict first, T3 delta | Strict second, T3 delta | Flex first, T3 delta | Turn-2 Item lock second, T3 delta |
|---|---:|---:|---:|---:|
| turbo/-Mawile+Crispin | +2.316 pp | +2.264 pp | +2.460 pp | +0.640 pp |
| turbo/-Mawile+Stevens | +0.244 pp | +4.036 pp | +0.120 pp | +0.928 pp |
| turbo/-Mawile,-Guzma+QuickBall,+Arven | +1.380 pp | +1.856 pp | +2.180 pp | +0.160 pp |
| turbo/-Mawile,-Tate+QuickBall,+Crispin | +1.708 pp | +2.464 pp | +2.332 pp | +0.444 pp |
| realism/-Mawile,-Tate+Burnet,+EarthenVessel | +1.132 pp | +2.032 pp | +1.460 pp | +1.844 pp |
| realism/-Mawile,-Tate+Burnet,+Arven | +1.500 pp | +1.624 pp | +1.560 pp | +1.480 pp |
| realism/-Tate+Burnet | -0.212 pp | +0.640 pp | -0.016 pp | +1.224 pp |
| realism/-Tate+EarthenVessel | +0.492 pp | +0.556 pp | +0.564 pp | +0.056 pp |
| turbo/-Mawile+EvolutionIncense | +0.988 pp | +1.652 pp | +1.004 pp | +0.540 pp |
| turbo/-Mawile+UltraBall | +0.200 pp | +0.692 pp | +0.756 pp | +0.288 pp |
| turbo/-Tate+UltraBall | -0.496 pp | -0.152 pp | -0.540 pp | -0.028 pp |

### Turbo speed recommendations

1. **Mawile-GX → Crispin.** This is the best one-card speed screen. It directly improves the GGF axis and avoids using the Supporter slot on a connector that only solves an earlier search axis. It improves strict-JIT T3 by +2.316 pp going first and +2.264 pp going second.
2. **Mawile-GX + Guzma → Quick Ball + Arven.** This is the deepest screened turbo package. It gains +1.380 pp strict first, +1.856 pp strict second, and +2.180 pp matchup-flex first. It also removes two high-discrete matchup cards, so this is suited to raw setup testing rather than blind ladder use.
3. **Mawile-GX + Tate & Liza → Quick Ball + Crispin.** This package gains +1.708 pp strict first and +2.464 pp strict second. It increases connector density while retaining Guzma.
4. **Mawile-GX → Steven’s Resolve.** The going-second improvement is strongest at +4.036 pp. Its going-first gain is only +0.244 pp, matching the first-turn Supporter restriction. Use this only when the expected line needs VSTAR, Crispin, and Blender together across turns.

### Realism-speed recommendations

1. **Mawile-GX + Tate & Liza → Professor Burnet + Earthen Vessel.** This is the strongest lock-focused screen: +1.844 pp by strict turn 3 and +2.032 pp in no-lock strict second. Burnet supplies an Item-lock-resistant discard route, while Vessel improves Energy access when Items are available.
2. **Mawile-GX + Tate & Liza → Professor Burnet + Arven.** This adds +1.500 pp strict first, +1.624 pp strict second, and +1.480 pp under turn-2 Item lock.
3. **Keep Mawile and preserve the whole discrete package.** The tested one-for-one Tate replacements do not establish a uniformly large gain. `Tate & Liza → Earthen Vessel` has a small strict-second gain, while `Tate & Liza → Professor Burnet` specializes toward the Item-lock case. The baseline remains the more defensible default where Mimikyu Copycat, gust, Tool removal, and late-game resource loops matter.

### Ultra Ball and Evolution Incense results

Ultra Ball was implemented with its actual two-hand-card payment. `Mawile-GX → Ultra Ball` gained only +0.200 pp strict first and +0.692 pp strict second. `Tate & Liza → Ultra Ball` was negative in the normal strict screens. Its raw Pokémon access loses to the payment problem in a singleton-heavy list.

Evolution Incense has no discard cost and solves the VSTAR axis cleanly. `Mawile-GX → Evolution Incense` gained +0.988 pp strict first and +1.652 pp strict second. It remains narrower than Mysterious Treasure, Arven, Forest Seal Stone, and Steven’s Resolve because it cannot locate a Basic, Energy bridge, payload outlet, or Supporter line.

### Complete variant screen

| Variant | Strict first | Strict second | Flex first | Turn-2 Item lock second |
|---|---:|---:|---:|---:|
| realism/-Mawile+Burnet | +0.724 pp | +1.472 pp | +0.668 pp | +1.564 pp |
| realism/-Mawile+Serena | +0.900 pp | +1.324 pp | +1.024 pp | +0.932 pp |
| realism/-Mawile,-Tate+Burnet,+Arven | +1.500 pp | +1.624 pp | +1.560 pp | +1.480 pp |
| realism/-Mawile,-Tate+Burnet,+EarthenVessel | +1.132 pp | +2.032 pp | +1.460 pp | +1.844 pp |
| realism/-Tate+Burnet | -0.212 pp | +0.640 pp | -0.016 pp | +1.224 pp |
| realism/-Tate+EarthenVessel | +0.492 pp | +0.556 pp | +0.564 pp | +0.056 pp |
| turbo/-FieldBlower+QuickBall | +0.092 pp | +0.612 pp | +0.068 pp | +0.180 pp |
| turbo/-Guzma+QuickBall | +0.316 pp | +0.516 pp | +0.256 pp | +0.056 pp |
| turbo/-Mawile+Arven | +1.396 pp | +1.560 pp | +1.692 pp | +0.204 pp |
| turbo/-Mawile+Crispin | +2.316 pp | +2.264 pp | +2.460 pp | +0.640 pp |
| turbo/-Mawile+EvolutionIncense | +0.988 pp | +1.652 pp | +1.004 pp | +0.540 pp |
| turbo/-Mawile+QuickBall | +0.800 pp | +1.240 pp | +0.820 pp | +0.208 pp |
| turbo/-Mawile+Stevens | +0.244 pp | +4.036 pp | +0.120 pp | +0.928 pp |
| turbo/-Mawile+UltraBall | +0.200 pp | +0.692 pp | +0.756 pp | +0.288 pp |
| turbo/-Mawile,-Guzma+QuickBall,+Arven | +1.380 pp | +1.856 pp | +2.180 pp | +0.160 pp |
| turbo/-Mawile,-Tate+QuickBall,+Crispin | +1.708 pp | +2.464 pp | +2.332 pp | +0.444 pp |
| turbo/-Mawile,-Tate+UltraBall,+Crispin | +1.732 pp | +1.952 pp | +1.624 pp | +0.316 pp |
| turbo/-Tate+UltraBall | -0.496 pp | -0.152 pp | -0.540 pp | -0.028 pp |

## 6. Strategic synthesis

### Best raw-speed direction

A third Crispin is the clearest single-slot setup-speed increase. The result reflects the actual bottleneck after an attacker chain exists: GGF must be achieved while preserving a Supporter for the correct turn. Crispin advances two Energy types across the attachment and hand axes.

### Best going-second package

A second Steven’s Resolve produces the strongest screened going-second turn-3 gain among one-card changes. It has a narrow job: a Regidrago V with incomplete VSTAR, energy, and Blender axes. The same route has limited going-first value because the Supporter cannot be used on the first player turn.

### Best lock-resilience direction

Professor Burnet is valuable specifically where an Item lock threatens the JIT payload window. It moves a chosen payload from deck to discard without an Item and without demanding a hand discard. Burnet cannot simultaneously accelerate Energy, so a Burnet package should be evaluated against the expected lock frequency rather than added solely because it has a positive lock delta.

### Cards the model deliberately refuses to call disposable

Mawile-GX, Guzma, Field Blower, Channeler, Path to the Peak, and the recovery suite carry board-state and matchup value after setup. A speed-only model sees those as low-frequency resources. The strict/matchup-flex distinction exists to avoid collapsing that discrete value into zero. The simulator records their DCI eligibility only in scenarios where the related matchup need is absent.

## 7. Validation checkpoints and remaining work

Completed checkpoints:

- Audited the exact selected card prints from the supplied corpus.
- Corrected an early turn-one evolution bug before final results.
- Added actual two-discard Ultra Ball logic, one-axis Evolution Incense comparison, and two-step Mysterious Treasure or Quick Ball payload lines.
- Verified CMake build and CTest invariants.
- Added locked, atomic output replacement for simulator results and audit output.
- Detected and corrected CSV escaping for comma-bearing variant labels before analysis.
- Regenerated the final baseline and variant CSV files after the corrections.

High-value next extensions:

1. Add an opponent state machine for Return Label, Lysandre Prism Star, Surprise Box, Girafarig, force-benching, Budew Item lock, and Mimikyu Copycat.
2. Add expected-value action selection for Serena and Tate & Liza instead of the present deterministic fallback policy.
3. Add per-card availability validation against the current Pokémon TCG Live client once a client-accessible legality export exists.
4. Add attack-phase game trees for Phantom Dive counter placement, Ryuno Glide and Powerglass recovery, Goodra damage reduction, Timeless-GX, and prize-race outcomes.
5. Add Eldegoss V, Giovanni’s Exile, Avery, Grant, Noivern, Appletun, Kirlia, and other proposed cards as audited modules before their swaps receive probability claims.

## Reproduction

```text
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build --output-on-failure
build\Release\regidrago_sim.exe --trials 100000 --variant-trials 25000 --seed 20260704 --out results\simulation_results.csv --variants-out results\variant_results.csv
```

On a non-multi-config generator, the executable is commonly `build/regidrago_sim` rather than `build\Release\regidrago_sim.exe`.
