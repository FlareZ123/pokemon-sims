# Five-Hand Trace Audit

## Method

The following deterministic hands were run through the exact executable with `--simulate-this`. Each trace is stored under `results/traces/`. The review criterion is **earliest legal Active Regidrago VSTAR with GGF plus a payload that satisfies the selected DCI/JIT profile**. The review treats a policy preference as invalid whenever it consumes a connector without advancing a missing setup axis.

Commands are reproducible with the named seed. For example:

```text
regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 6 --require-ready-by 3
```

The trace prints Prize cards only as a debugging aid. The policy does not inspect them before a legal effect reveals or searches them.

## Reviewed hands

| Case | Command / saved trace | First ready | Manual legality and speed review |
|---|---|---:|---|
| Strict JIT, going second | `strict-jit/go-second`, seed `6` / `strict_jit_go_second_seed_6.txt` | T3 | Turn 1 has Quick Ball plus only protected payloads and required Energy. Strict JIT correctly refuses to spend a payload early. Turn 2 Quick Ball discards Mega Dragonite ex only after the JIT window is open, finds Regidrago V, and attaches Grass. Turn 3 supplies the remaining GGF, evolves, discards Goodra VSTAR with Mysterious Treasure in the ready turn, then uses Latias to retreat the Basic Active. T2 is unavailable because neither Regidrago V nor a legal early strict payload outlet exists on T1. |
| Strict JIT, going first | `strict-jit/go-first`, seed `2` / `strict_jit_go_first_seed_2.txt` | T4 | T1 correctly has no draw, Supporter, or attack. Mysterious Treasure gets VSTAR. T2 Tapu Lele-GX finds Crispin; Crispin plus manual attachment gives GF and evolves. Legacy Star is used only as the remaining potential T2 payload line and misses. T3 reaches GGF but lacks a current-turn payload outlet. T4 Arven finds Brilliant Blender and Blender discards Mega Dragonite ex. The review found no legal T3 payload outlet remaining, so T4 is the fastest modeled line. |
| Matchup-flex JIT, going second | `matchup-flex-jit/go-second`, seed `4` / `matchup_flex_go_second_seed_4.txt` | T2 | The line spends Guzma and Field Blower as explicit flex discard fodder to get VSTAR and Tapu Lele-GX, then Wonder Tag finds Crispin. Grass manual plus Crispin provides two attack-paying Energy before the turn-1 Celestial Roar. T2 attaches Fire, evolves, then Legacy Star places Goodra VSTAR into the current-turn discard. T2 is the earliest possible deadline. The Celestial Roar cost check requires at least one Energy and is satisfied. |
| Strict JIT, turn-two Item lock, going second | `strict-jit-turn2-item-lock/go-second`, seed `9` / `strict_turn2_item_lock_go_second_seed_9.txt` | T3 | T1 plays Tapu and Oricorio while their entry Abilities are live. Steven’s Resolve fetches VSTAR, Crispin, and **Professor Burnet**, rather than Blender, because Items lock on T2. T2 makes GGF and evolves but Crispin has consumed the Supporter. T3 Burnet supplies the same-turn payload. This is the correct bridge under the modeled lock and no current legal T2 Supporter remains for Burnet. |
| Strict JIT, Rule Box Ability lock, going second | `strict-jit-rulebox-ability-lock/go-second`, seed `19` / `strict_rulebox_lock_go_second_seed_19.txt` | T3 | The lock suppresses Rule Box Abilities, so Tapu Lele-GX, Latias ex, and Legacy Star are unavailable. T1 Gladion legally reveals Prizes only after play and retrieves prized VSTAR. Regidrago V has an Energy attached before Celestial Roar. T2 Crispin completes GGF and evolves; T3 Burnet puts Mega Dragonite ex in discard. Playing Crispin on T1 would strand VSTAR in Prizes, so the Gladion-first line is the fastest legal route. |
| No-discard-control, going first | `no-discard-control/go-first`, seed `6` / `no_discard_control_go_first_seed_6.txt` | T4 | T1 Quick Ball may discard Mega Dragonite ex because this profile permits early payload banking. It gets Regidrago V and attaches Grass. T2 attaches the second Grass, evolves, and uses Latias to promote VSTAR. The missing Fire does not appear until Crispin on T4. The lack of a T3 Fire or Crispin connector makes the T4 deadline unavoidable in the modeled line. |

## Fixes prompted by the trace pass

The audit changed the engine before accepting these lines:

1. Corrected first-turn draw, Supporter, and attack restrictions.
2. Added Regidrago V Celestial Roar with its one-Colorless attack cost. It cannot be used by an unpowered Regidrago V.
3. Added Legacy Star with the shared VSTAR-Power limit and current-turn discard accounting.
4. Prevented opening setup from silently consuming Tapu Lele-GX and Oricorio entry Abilities.
5. Preserved Oricorio under Rule Box Ability lock because it has no Rule Box.
6. Corrected Earthen Vessel same-type Energy selection and Crispin’s different-type/attachment condition.
7. Corrected Gladion’s mandatory Prize exchange and removed pre-reveal Prize oracle behavior.
8. Removed early strict-JIT payload banking while allowing it in the no-discard-control reference scenario.
9. Added Tate & Liza switch mode and Latias’s Basic-only free-retreat condition.
10. Added connector-domination checks for Oricorio, Tapu Lele-GX, Heavy Ball, Arven, Gladion, and Steven’s Resolve.
11. Fixed two test-only lifetime defects and a repeated-Energy search accounting defect found by AddressSanitizer and UndefinedBehaviorSanitizer.

## Validation

The exact branch source passed both matrices after the final Celestial Roar cost correction:

```text
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

Eight tests pass: a self-test, the six deterministic trace regressions in the table, and a 14-scenario aggregate smoke test.

A second build used `-fsanitize=address,undefined` and ran the same eight tests with `ASAN_OPTIONS=detect_leaks=1`.

## Remaining boundary

This is still a goldfish setup policy engine, not a complete two-player Pokémon TCG rules engine. It intentionally does not resolve opponent board effects, damage, Knock Outs, Prize taking, hand disruption, gust, stadium sequencing beyond the exogenous lock scenarios, Mimikyu Copycat, Return Label, Lysandre Prism Star, or every printed card in Expanded. Those interactions are deliberately outside these traces and must not be inferred from their absence.
