# Simulator Audit: `fix-sims`

## Status

This branch corrects material card-rule, hidden-information, policy, and traceability defects found in the simulator merged to `main`. The active executable is `src/regidrago_sim.cpp`; it is assembled from `src/trace_engine_parts/` solely to keep the GitHub contents-API commits reviewable.

The branch now has a deterministic `--simulate-this` mode, a full rules-to-code register in [`RULES_TRACEABILITY.md`](RULES_TRACEABILITY.md), and six manually reviewed paths in [`TRACE_AUDIT.md`](TRACE_AUDIT.md). `main` remains unchanged.

## Main corrective themes

1. **Game procedure.** Corrected going-first draw, Supporter, and attack restrictions; evolution timing; manual attachment limit; retreat limit; and Regidrago V's one-Colorless Celestial Roar cost.
2. **Entry effects and locks.** Tapu Lele-GX and Oricorio stay in hand at opening setup until they can be played during a turn. Oricorio is not a Rule Box Pokémon, so the modeled Path-style lock does not block Vital Dance.
3. **Energy rules.** Earthen Vessel can choose two copies of the same Basic Energy. Crispin needs two different Energy types to use its attachment branch and attaches none when only one type can be found.
4. **Hidden information.** Gladion, Hisuian Heavy Ball, and discard policy decide from known zones and copy counts, then inspect deck or Prize cards only after the card effect begins.
5. **JIT and discard control.** Strict and matchup-flex profiles require current-turn payload discard. No-discard-control permits early payload banking. Dead Dipplin is an explicit legal DCI discard because the list contains no Applin.
6. **Connector planning.** The policy avoids dominated searches and Supporter lines, preserves direct connectors, gives normal Basic search priority over Heavy Ball, avoids current Item-lock Arven, and has Steven's Resolve fetch Burnet rather than Blender when the next attack turn is Item-locked.
7. **Position and VSTAR powers.** Tate & Liza switch mode and Latias's Basic-only free retreat are modeled. Legacy Star and Forest Seal Stone share the single VSTAR-Power state.
8. **Search resolution.** Mandatory searches resolve with legal fallback targets after the deck is inspected. The energy selector reserves each selected copy so it cannot choose the same last card twice.

## Trace and validation contract

The deterministic command format is:

```text
regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 6 --require-ready-by 3
```

The trace prints every draw, cost, search, bench, attachment, evolution, retreat, VSTAR Power, attack, and ready state. Each state-changing line names the controlling `R-*` rule IDs and any policy IDs.

The final validation matrix is:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

CTest includes one self-test, six deterministic trace regressions across strict/flex/no-control and lock scenarios, and a fourteen-scenario aggregate smoke run. The same matrix was run after an AddressSanitizer and UndefinedBehaviorSanitizer build.

## Results status

`results/simulation_results.csv` was regenerated from the trace-audited engine at 100,000 trials per scenario with seed `20260705`. The old pre-trace variant artifact was removed and must not be used for deck recommendations until regenerated from this engine.

## Scope boundary

This remains a single-player setup policy model, not a complete two-player Pokémon TCG engine. It does not model opponent damage, Knock Outs, Prize taking, hand disruption, gust, Return Label, Lysandre Prism Star, Surprise Box, Mimikyu Copycat, complete Stadium sequencing, or exhaustive Expanded-format legality.
