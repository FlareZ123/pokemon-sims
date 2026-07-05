# Simulator Audit: `fix-sims`

## Status

This branch corrects material rules, hidden-information, and policy errors found in the simulator that was merged to `main`. The corrected executable is `src/regidrago_sim_fixed.cpp`, assembled from the reviewed fragments in `src/fixed_parts/` so the audit diff stays readable.

`main` is intentionally unchanged. The prior result artifacts and report values are superseded on this branch.

## Corrected defects

### 1. Opening-Bench entry Abilities were lost

The old setup routine automatically Benched Tapu Lele-GX and Oricorio during opening setup. Their effects require playing the Pokémon from hand to Bench during a turn. The previous model removed those cards from hand without resolving Wonder Tag or Vital Dance, lowering or distorting access in ways that depended on the opening hand.

The revised opening plan holds Tapu Lele-GX and Oricorio in hand. It only opens an additional Regidrago V and a useful Latias ex, then plays Tapu or Oricorio later when their entry effect is live.

### 2. Rule Box lock incorrectly stopped Oricorio

The old Ability-lock helper disabled every Ability. Path to the Peak-style lock applies to Pokémon with a Rule Box. Oricorio GRI 55 has no Rule Box, so Vital Dance remains active.

The revised engine distinguishes Rule Box Pokémon from non-Rule-Box Pokémon. It continues to suppress Tapu Lele-GX, Latias ex, Regidrago VSTAR and other Rule Box Abilities under the modeled lock, while leaving Oricorio active.

### 3. Earthen Vessel could not select two Grass Energy

The earlier code always took one Grass and one Fire. Earthen Vessel allows up to two Basic Energy cards and has no different-type clause. Two Grass is legal and often correct when Regidrago still needs both Grass attachments.

The corrected search selector chooses a G/F split when both requirements exist, two Grass when the remaining need is two Grass, and equivalent legal fallbacks after the deck is inspected.

### 4. Crispin could attach an illegal lone Energy

Crispin searches for up to two Basic Energy cards of different types, puts one into hand, and attaches the other. The old model attached a single found Energy when only one type remained in the deck. That cannot happen because the attachment requires the two-card different-type selection.

The revised resolver attaches only after it finds both a Grass and a Fire. A one-card degraded resolution places the found Energy into hand and attaches none.

### 5. Prize and deck information leaked into decisions

The previous policy looked directly at exact Prize contents before deciding to play Gladion or Hisuian Heavy Ball. It also used the exact remaining deck to call an Energy disposable before any effect inspected the deck. Those are hidden-information leaks.

The new decision policy uses visible hand, board, discard, and known copy counts to decide whether to play a card. It inspects the deck or Prize cards only after the relevant legal search or reveal effect has started.

### 6. The non-control reference line blocked legal turn-one payload banking

The old `NoDiscardControl` scenario still required turn 2 before Brilliant Blender or Professor Burnet could discard a payload. That invalidated the optimistic reference scenario and understated the JIT tax.

The revised `payload_window_open` permits early payload banking in the no-discard-control profile, while strict and matchup-flex JIT still require the payload to enter discard during the attack turn.

### 7. Tate & Liza switch mode was absent

The old simulation only implemented Tate & Liza's shuffle-and-draw mode. It omitted the active-position line that switches a Benched Regidrago VSTAR into the Active Spot.

The fixed policy uses the switch mode when it resolves the immediate Active VSTAR axis, preserving the draw mode for low-hand fallback states.

### 8. Connector domination and Supporter planning were weak

Several actions could consume scarce resources without advancing a required setup axis:

- Hisuian Heavy Ball could fire before ordinary live Basic-search routes.
- Gladion could be spent despite an available search or Forest Seal Stone route.
- Arven could be played under Item lock despite producing only unusable Item and Tool cards in the setup model.
- Steven's Resolve could fetch Brilliant Blender for a turn where the configured turn-two Item lock made Blender unusable.
- Search effects could stop after a preferred card was missing, despite legally selecting another useful target after examining the deck.

The revised policy gates those decisions on the missing axis, gives higher-priority live connectors their chance first, avoids current Item-lock Arven, uses Professor Burnet in Steven's package when the next attack turn will be Item-locked, and applies fallbacks after legal searches.

### 9. Early-board planning over-benched Regidrago V

The original opening policy could create an Active Regidrago V plus two Bench Regidrago V, spending three slots on the same chain before any matchup information existed. The corrected plan caps the early Regidrago V plan at two total copies, retaining Bench space for later Tapu Lele-GX, Oricorio, Latias ex, and recovery lines.

### 10. Serena and effect-level shuffling were incomplete

The revised model requires Serena's mandatory first discard and permits additional Serena discards only for expressly matchup-flex fodder. It also performs deck shuffles at the end of each relevant search effect rather than after each selected card.

## Validation completed

The repaired branch was checked with:

```text
cmake -S . -B build-split -DCMAKE_BUILD_TYPE=Release
cmake --build build-split --config Release
ctest --test-dir build-split --output-on-failure
```

Both CTest targets passed:

- `regidrago_sim_self_test`
- `regidrago_sim_smoke`

An AddressSanitizer and UndefinedBehaviorSanitizer run also completed successfully after the initial null-Regidrago Crispin crash was corrected.

The final result artifacts were regenerated with:

```text
regidrago_sim --trials 100000 --variant-trials 25000 --seed 20260705 --out results/simulation_results.csv --variants-out results/variant_results.csv
```

## Corrected baseline highlights

| Scenario | Ready by T2 | Ready by T3 | Ready by T4 |
|---|---:|---:|---:|
| Strict JIT, going first | 6.592% | 15.268% | 22.126% |
| Matchup-flex JIT, going first | 6.417% | 19.144% | 29.043% |
| No discard control, going first | 9.573% | 28.407% | 41.902% |
| Strict JIT, going second | 14.800% | 22.110% | 27.332% |
| Matchup-flex JIT, going second | 18.842% | 27.411% | 34.656% |
| No discard control, going second | 17.507% | 33.644% | 44.219% |

The stricter profiles now have higher turn-two readiness than the former report because legal Oricorio entry lines, correct Energy selection, correct opening-hand retention, Active-position switching, and corrected connector planning are represented. The non-control line also rises because turn-one Blender and Burnet banking is now legal in that scenario.

## Remaining scope boundary

This repair improves the legal setup-state model. It remains a single-player policy simulator. It does not yet solve opponent-dependent attack selection, damage, prize maps, Return Label, Lysandre Prism Star, Surprise Box, Mimikyu Copycat, gust, hand disruption, full Recovery-loop planning, or a two-player imperfect-information game tree.
