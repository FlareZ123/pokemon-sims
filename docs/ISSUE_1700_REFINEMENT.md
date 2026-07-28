# Issue 1700 refinement: registered-shell witnesses

Issue: https://github.com/FlareZ123/pokemon-sims/issues/1700

Audited source: `main@fdfd5dca1a46fa6160a43da1de472a3c80ff7cd2`.

The original `crobat1-heavy-ball`, `matchup-flex-jit/go-second`, seed `218` witness delays deterministic readiness from T3 to T4 because the policy holds Earthen Vessel and leaves the T1 manual attachment unused before Steven's Resolve ends the turn.

The same underlying pre-Steven Item and attachment ordering defect affects the registered `regidrago-shell` recipe at seed `218`.

## Modeling witness target correction

The original modeling-route description must have Steven's Resolve search both Latias ex and a second Grass Energy after the pre-Steven Earthen Vessel searches Grass plus Fire and the first Grass is manually attached on T1. Searching only Latias ex would leave no held Grass for the T2 manual attachment.

The corrected deterministic T3 modeling line is:

1. After Mysterious Treasure Benches Regidrago V, play Earthen Vessel before Steven's Resolve.
2. Discard Mega Dragonite ex, search Grass Energy plus the only deck Fire Energy, and manually attach Grass to Regidrago V on T1.
3. Play Steven's Resolve for Latias ex plus a second Grass Energy, then end T1.
4. T2: evolve Regidrago V, manually attach the second Grass, Bench Latias ex, retreat the Basic Active through Skyliner, and promote Regidrago VSTAR.
5. T3: manually attach the held Fire Energy, play Brilliant Blender for a current-turn matchup-flex-JIT payload, and reach `READY`.

This correction preserves the original bug diagnosis and removes an impossible held-Energy assumption from the proposed route.

## Registered-shell strict and matchup-flex JIT

Commands:

```text
./build/regidrago_sim --simulate-this --deck regidrago-shell --scenario strict-jit/go-second --seed 218
./build/regidrago_sim --simulate-this --deck regidrago-shell --scenario matchup-flex-jit/go-second --seed 218
```

Current policy first establishes K1, uses Mysterious Treasure to Bench Regidrago V, leaves Earthen Vessel and the T1 manual attachment unused, then searches redundant Regidrago V, Regidrago VSTAR, and Gladion with Steven's Resolve. All three Fire Energy are known prized. The simulator reaches `GG`, never recovers Fire, and records a setup failure through T5.

A deterministic T3 route exists:

1. After Mysterious Treasure Benches Regidrago V, play Earthen Vessel before Steven's Resolve.
2. Under strict or matchup-flex JIT, discard the held Regidrago VSTAR as the realistic cost. Steven can immediately replace it, preserving the current-turn Blender payload requirement.
3. Search two Grass Energy and manually attach one Grass to Regidrago V on T1.
4. Play Steven's Resolve for Regidrago VSTAR, Latias ex, and Gladion, then end T1.
5. T2: evolve, manually attach the second Grass, Bench Latias ex, play Gladion for a known-prized Fire Energy, and retreat through Skyliner.
6. T3: manually attach Fire Energy, play Brilliant Blender for a current-turn permitted Dragon payload, and reach `READY`.

The route uses only K1 information and reaches the earliest possible ready turn. Three Energy attachments cannot be completed by T2 in this state.

## Registered-shell no-discard-control

Command:

```text
./build/regidrago_sim --simulate-this --deck regidrago-shell --scenario no-discard-control/go-second --seed 218
```

Current policy first reaches readiness on T4. The same T1 Vessel and attachment route reaches T3. In this profile Mega Dragonite ex is a realistic Vessel cost because the payload may enter discard before the ready turn. Steven then searches Regidrago VSTAR, Latias ex, and Gladion; T2 supplies the second Grass and retrieves Fire; T3 attaches Fire and reaches readiness.

## Sources

Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163

Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145

Gladion: https://api.pokemontcg.io/v2/cards/sm4-95

Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76

Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164

Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136

Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152

Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf

Repository knowledge, DCI, JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#discard-capability-index-dci https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities

Current issue-1645 route override: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1030_steven_turo_override.inc#L192-L268

## Refined fix scope

The eventual confirmed fix should project the pre-Steven Vessel line for both registered and modeling recipes. It must select a realistic DCI cost for the active profile, search the legal Energy package, use the T1 manual attachment, and choose Steven targets that complete the remaining VSTAR, Active-position, Prize-recovery, and payload axes. Negative controls must preserve current behavior when any required cost, Energy, Basic, evolution, Latias, Gladion, Prize identity, Bench slot, retreat route, or current-turn payload outlet is unavailable.
