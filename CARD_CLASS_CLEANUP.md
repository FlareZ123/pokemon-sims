# Card Class Cleanup

This file is the live architecture and migration plan. Historical cleanup-wave notes belong in Git history. Keep this document limited to current ownership, remaining work, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Reference seam

Quick Ball remains the reference card-class migration. Preserve this path before starting another card migration:

```text
src/cards/card_id.hpp
src/cards/card_definition.hpp
src/cards/card_registry.hpp
src/cards/trainers/quick_ball.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/adapters/card_context_adapter.hpp
src/trace_engine_v2/core/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball owns the reference behavior for exact-print registration, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Current ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokemon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration and canonical definition lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations and the shared `CardContext::Callbacks` / `CardContext::Classifiers` bundles.
- `src/trace_engine_v2/core/adapters/card_context_adapter.hpp` owns the trace-engine bridge and aliases `CardContextAdapterCallbacks` to the rules-owned callback bundle.
- `src/trace_engine_v2/core/card_context_adapter.hpp` remains a forwarding compatibility include until every direct consumer uses the organized adapter owner.
- `src/trace_engine_v2/core/card_catalog.inc` owns unmigrated name and intrinsic-classification fallbacks. Registry metadata remains the first lookup path.
- Engine strategy owns route admission, target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.

The callback-shape cleanup is complete: the rules layer now owns one `CardContext::Callbacks` aggregate, the trace adapter reuses that type, and the live adapter constructor no longer mirrors the callback fields. Preserve this single ownership point when adding future card operations.

## Next adapter work

1. Migrate direct includes of `src/trace_engine_v2/core/card_context_adapter.hpp` to `core/adapters/card_context_adapter.hpp` when their complete seams are touched.
2. Delete the forwarding compatibility header after repository-wide source and tooling references are proven gone.
3. Extend `CardContext::Callbacks` or `CardContext::Classifiers` only for reusable printed-rule operations. Keep route policy in Engine.
4. Keep callback construction named. Do not reintroduce positional callback lists.

Canonical adapter: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/adapters/card_context_adapter.hpp
Rules context: https://github.com/FlareZ123/pokemon-sims/blob/main/src/rules/card_context.hpp

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, move metadata and classification first. Move printed resolution after identifying the live resolver and reusable `CardContext` operations. Keep strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata and classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If a migration exposes incorrect gameplay behavior, use the bug-confirmation workflow and keep the cleanup PR behavior-preserving.

## Catalog cleanup

Migrate `LegacyCardCatalog` and intrinsic fallback entries one card at a time. Delete a compatibility row after that card has an explicit `CardDefinition`, deterministic registration, exact-print source, and focused metadata test.

Regidrago V and Regidrago VSTAR are the reference Pokemon metadata migration. Exact prints: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136

## Composition cleanup

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It owns simulator runtime inclusion, the opening `part_003.inc` to `part_005.inc` continuation, banked-Tapu and lock-removal alias lifetimes, and the late `part_014c.inc` to `part_016.inc` continuation. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

Mechanical `.inc` cleanup must preserve macro setup/teardown order, declaration order, member boundaries, and relative include roots. Route admission, projection, and decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

Current canonical route owners include:

- Crispin-supported completion: `src/trace_engine_v2/core/routes/crispin_supported_route_policy.inc`
- Quick Ball / Tapu Lele-GX / Crispin: `src/trace_engine_v2/core/routes/quick_ball_tapu_crispin_policy.inc`
- Earthen Vessel / Celestial Roar: `src/trace_engine_v2/core/routes/earthen_vessel_celestial_roar_policy.inc`
- Banked Tapu retreat: `src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc`

Retire another root `part_*` seam only when its complete macro lifetime or function body can move intact and repository-wide tooling references have been checked.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner. Prefer its zone traversal, membership, count, and explicit preference helpers over route-local `std::find_if`, `std::any_of`, or `std::count_if` copies when the semantics are identical.

Next payload step: audit remaining route-local Dragon-payload scans. Consolidate only exact semantic duplicates. Keep physical-zone order where historical behavior depends on it and keep strategic priority explicit.

Canonical payload owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136

## Validation gate

Every cleanup PR must remain behavior-preserving and pass:

1. Release build and strict C++20 contract.
2. Unified policy/card tests.
3. ASan and UBSan suite.
4. Structural/source-contract checks.
5. Representative `--simulate-this` trace audits.
6. Paired T2/T3 setup matrix generation and verification.

Rules-sensitive lines keep direct card, ruling, advanced-manual, or repository-spec URLs beside the code. Cleanup that changes gameplay semantics must be split into the normal confirmed-bug workflow.
