# Card Class Cleanup

This file is the live architecture and migration plan. Historical cleanup-wave notes belong in Git history. Keep this document limited to current ownership, remaining work, and validation requirements.

## Operating rule

> **Strategy chooses. Card code validates and resolves. Rules primitives perform state transitions.**

Preserve this dependency direction:

```text
rules <- cards <- simulator/strategy
```

Code under `src/cards/` must not include trace-engine implementation files or inspect raw `Engine` or `State` data.

## Bootstrap gate

Do not begin another card migration unless the Quick Ball reference seam remains intact:

```text
src/cards/card_id.hpp
src/cards/card_definition.hpp
src/cards/card_registry.hpp
src/cards/trainers/quick_ball.hpp
src/rules/card_context.hpp
src/trace_engine_v2/core/card_context_adapter.hpp
src/trace_engine_v2/core/quick_ball_card_class_base.inc
src/trace_engine_v2/core/quick_ball_card_class_tail.inc
tests/quick_ball_card_class_tests.cpp
```

Quick Ball is the reference for explicit registration, exact-print metadata, intrinsic cost validation, K0 to K1 search timing, strategy-owned target choice, printed target filtering, source-card movement, failed-search behavior, shuffle, and trace compatibility. Exact print: https://api.pokemontcg.io/v2/cards/swsh1-179

## Architecture ownership

- `src/cards/card_id.hpp` owns stable `sim::Card` identifiers. Exact external print identity belongs in `CardDefinition::canonical_id`.
- `src/cards/card_definition.hpp` owns intrinsic exact-print facts such as name, print ID, Trainer subtype, stage/type, Retreat Cost, Rule Box/Pokémon V/ACE SPEC/Basic Energy flags, and direct source URL.
- `src/cards/card_registry.hpp` owns explicit deterministic registration. `kRegisteredCardDefinitions` is the canonical inventory and `find_definition()` is the canonical lookup: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
- `src/rules/card_context.hpp` owns reusable printed-rules operations. Card-specific route policy stays outside that interface.
- Engine strategy owns route admission, strategic target preference, DCI/UDP/AMR, strict-JIT and matchup-flex timing, Supporter contention, connector domination, K0/K1 state, setup-axis value, lock schedules, readiness, and payload policy.
- `src/trace_engine_v2/core/card_catalog.inc` is the compatibility owner for unmigrated names and intrinsic classification fallbacks. Registry lookup remains the first and canonical metadata path. The former sibling `core/card_classification.inc` has been merged into this owner so the textual catalog/classification continuation cannot drift apart.

Next catalog step: migrate remaining `LegacyCardCatalog` and intrinsic compatibility entries one card at a time through the normal ownership workflow. Delete a compatibility row only after that card has an explicit `CardDefinition`, registration, exact-print source, and focused metadata test. Keep gameplay resolution and strategy at their current owners during metadata-only migrations.

Regidrago VSTAR now owns exact Silver Tempest 136/195 metadata beside Regidrago V in `src/cards/pokemon/regidrago_v.hpp`, is explicitly registered, and has focused V/VSTAR metadata/parity coverage. The live Pokémon, Pokémon V, Rule Box, Dragon/Mysterious Treasure target, and Retreat Cost classifiers consume registered metadata for the Regidrago line. Exact print: https://api.pokemontcg.io/v2/cards/swsh12-136 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Pokémon V ruling: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/

The shadowed Regidrago VSTAR compatibility name row has now been retired from `LegacyCardCatalog`; `CardDefinition` and the registry are the sole name owner for that print. Registered Pokémon Retreat Cost lookup also consumes `CardDefinition::retreat_cost` generically, leaving the legacy switch only for Pokémon that have not migrated. Keep V -> VSTAR evolution/devolution relations, Legacy Star, Apex Dragon, payload policy, DCI/JIT, and route choice at their existing behavioral owners. Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Retreat procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## Active card migrations

Do not create a parallel migration while one of these owners is active:

- Erika's Invitation: https://github.com/FlareZ123/pokemon-sims/issues/3598 Exact print: https://api.pokemontcg.io/v2/cards/sv3pt5-160
- Guzma: https://github.com/FlareZ123/pokemon-sims/issues/3618 Exact print: https://api.pokemontcg.io/v2/cards/sm3-115
- Gladion: https://github.com/FlareZ123/pokemon-sims/issues/3604 Exact print: https://api.pokemontcg.io/v2/cards/sm4-95
- Team Yell's Cheer: https://github.com/FlareZ123/pokemon-sims/issues/3620 Exact print: https://api.pokemontcg.io/v2/cards/swsh9-149

For each migration, metadata/classification can move first. Printed resolution moves only after the live resolver and general `CardContext` operations are identified. Strategic selection, DCI/UDP/AMR, Supporter contention, connector domination, K0/K1 handling, and lock policy remain in Engine. Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

## One-card workflow

1. Search open issues for an existing migration owner.
2. File and claim a migration only when unowned.
3. Classify every `Card::<Name>` occurrence as metadata, printed effect, rules transition, strategy, test, or documentation.
4. Add one primary card module and register it explicitly.
5. Move intrinsic metadata/classification ownership first.
6. Locate the single live printed-resolution owner before moving state transitions.
7. Preserve K0/K1 timing and keep strategic target choice in Engine.
8. Add focused tests for metadata and printed legality/effect boundaries.
9. Run strict CI, representative `--simulate-this` traces, and the paired T2/T3 matrix before merge.

If migration exposes gameplay behavior that is wrong, use the normal bug-confirmation workflow instead of combining the fix with cleanup.

## Composition ownership

`src/trace_engine_v2/composition/engine_body.inc` is the canonical ordered Engine composition owner. It now directly owns the simulator runtime inclusion and formatting continuation formerly forwarded through `core/simulator_state.inc`, the complete opening `part_003.inc` -> `part_004.inc` -> `part_005.inc` alias lifetime, the banked-Tapu and lock-removal alias setup/include/teardown lifetimes, and the late `part_014c.inc` -> `part_015.inc` -> `part_016.inc` alias lifetime at their established textual boundaries. The forwarding-only `simulator_state.inc`, `opening_legacy_stage.inc`, `late_engine_stage.inc`, `banked_tapu_policy_stage.inc`, `lock_removal_policy_stage.inc`, and `opening_state_completion_stage.inc` layers have been retired without changing their textual continuation or `#define` / `#include` / `#undef` order. Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc Runtime state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc

The opening boundary keeps `begin_turn`, opening-deck visibility, and the Garbodor Ability alias adjacent to the historical fragments that consume them. The late boundary keeps `play_field_blower`, `run_turn`, scenario-registry aliases, and the final translation-unit closure adjacent for the same reason. Historical opening fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc Historical late fragment: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

The banked-Tapu and lock-removal policy implementations remain separate semantic owners under `core/`, while composition-only macro lifetimes stay in `engine_body.inc`. This keeps route and lock policy ownership distinct without retaining forwarding-only composition files. Banked-Tapu route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forest_field_blower_policy.inc

Mechanical `.inc` cleanup must preserve `#define` / `#include` / `#undef` order, declaration order, member boundaries, and relative include roots. Route admission/projection/decision policy stays under `src/trace_engine_v2/core/routes/`. C++ textual-include semantics: https://eel.is/c++draft/cpp.include

The former `core/card_catalog.inc` -> `core/card_classification.inc` textual pair is now one `core/card_catalog.inc` owner. `engine_body.inc` preserves the historical classification feature marker without a second include, while names, registered-metadata lookup, and legacy intrinsic fallback predicates now live in one compatibility seam. Canonical merged owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

The root `part_000.inc` and `part_001.inc` compatibility paths remain only because unified-test/source-contract tooling reads them directly. Their historical source-anchor padding has been collapsed to minimal canonical shims: `part_000.inc` exposes the catalog include needed by unified-test header discovery, while `part_001.inc` preserves the non-executable payload predicate mirror expected by raw-source contracts. Neither file owns executable card behavior. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc Unified-test generator: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/engine_body.inc

`part_issue_1673_secret_box_deadline_override.inc` now owns the immediately following Professor Turo late-promotion continuation at the same textual member boundary. The former forwarding continuation `part_issue_1674_turo_late_promotion_override.inc` has been retired after moving its helpers and `run_turn_issue1675_original()` definition unchanged beside the Secret Box deadline continuation. Preserve this declaration order and keep the next Latias continuation include at the same tail boundary. Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163 Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171 Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1673_secret_box_deadline_override.inc

The same composition owner now also contains the issue-1724 Crobat V Stadium-compression retry beside `run_turn_issue1674_original()`, its sole caller. The one-purpose `part_issue_1724_crobat_stadium_compression.inc` fragment has been retired while preserving the function body, declaration order, call order, and direct rules/card URLs. Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104 Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187 Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1724

`post_014a_overrides.inc` now composes `core/routes/issue_1016_legacy_star_quick_ball_policy.inc` directly under the same `use_legacy_star` alias lifetime that previously wrapped `part_issue_1016_legacy_star_quick_ball_override.inc`. The forwarding-only root shim has been retired without moving the alias boundary or changing route policy. Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179 Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136 Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_1016_legacy_star_quick_ball_policy.inc Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

The issue-3199 public-projection isolation and Tate/Turo projection guard now live directly in `post_014a_overrides.inc` at their previous textual positions. The forwarding-only `part_issue_1069_legacy_star_combined_energy_payload_override.inc` and `part_turo_oricorio_override.inc` layers are retired. Their macro setup, thread-local projection depth, RAII guard, and canonical route includes are unchanged apart from the relative include roots required by the composition directory. Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136 Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148 Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc Canonical Legacy Star route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_3199_legacy_star_combined_energy_payload_base.inc Canonical Tate/Turo route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_3199_turo_oricorio_base.inc

The final Arven FSS/Blender contention continuation now lives directly in `post_014a_overrides.inc` under the same `play_arven -> play_arven_empty_deck_original` alias lifetime that previously included `part_012_arven_fss_blender_contention_override.inc`. The one-purpose root fragment is retired, and its route logic plus direct rule/card URLs remain at the identical textual boundary. Arven: https://api.pokemontcg.io/v2/cards/sv1-166 Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156 Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164 Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

`src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc` is the semantic owner for the complete Professor Burnet ready-turn admission and resolver body. `post_014a_overrides.inc` now composes that owner directly at the established `play_professor_burnet -> play_professor_burnet_empty_deck_original` alias boundary, and the forwarding-only `part_011_burnet_thinning_override.inc` compatibility shim has been retired. Macro lifetime, declaration order, Supporter contention, JIT/DCI policy, and the direct card/rule URLs remain at their established semantic and composition owners. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc Composition owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

Within that semantic owner, one-card finishing-Energy admission uses a boolean projection because callers consume only route existence, the ordered payload and setup-dead thinning choices have named local priority tables, the printed two-card Burnet selection limit is named once, and deck-to-discard resolution is centralized through one local transition helper. Keep these order-sensitive Burnet strategy lists and the shared discard transition inside the Burnet owner unless another proven route needs exactly the same semantics. Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26

Next composition step: inspect another remaining root `part_*` seam only when it is live or its complete macro lifetime/function body can move intact into an existing semantic owner at the identical textual boundary. Keep tooling-only compatibility paths minimal, and retire them only after their generator/source-contract consumers migrate to canonical owners. Preserve declaration order, route semantics, and direct source URLs, and do not recreate forwarding-only sequencers. For metadata-only continuation pairs, prefer merging adjacent intrinsic owners when their declaration order and dependencies are already identical, as with the retired catalog/classification executable seam and Professor Burnet forwarder.

## Payload policy cleanup

`src/trace_engine_v2/core/payload_hand_policy.inc` is the canonical Dragon-payload query owner.

- `PayloadZonePolicy::first_iterator_matching()` owns the shared physical-zone first-match traversal primitive. Payload and exact-card membership build on this seam so they cannot drift into separate scan implementations.
- `PayloadZonePolicy::contains_matching()` owns generic predicate-based zone membership and keeps boolean membership checks on the same traversal primitive.
- `PayloadZonePolicy::count_matching()` owns generic predicate-based zone cardinality so payload counts do not grow independent `std::count_if` scans.
- `PayloadZonePolicy::first()` preserves physical zone order for callers whose historical behavior depends on the first matching payload.
- `PayloadZonePolicy::contains()` and `PayloadZonePolicy::count()` own generic payload membership/count semantics.
- `PayloadZonePolicy::contains_card()` owns concrete-card membership in a physical zone so preference code does not duplicate `std::find` scans.
- `PayloadPreferencePolicy::first_preferred()` preserves the explicit five-card strategic priority.
- `PayloadPreferencePolicy::first_preferred_in_zone()` composes preference order with physical-zone membership.
- `PayloadPreferencePolicy::first_preferred_with_positive_count()` adapts count-backed zones without duplicating preference traversal.

The #2408 Burnet-versus-Serena held-Dragon check now delegates to `payload_zone_contains(state_.hand)`. The #2271 surplus-Regidrago route now delegates its exclusion-aware hand scan to `PayloadZonePolicy::contains_matching()` while preserving the `Card::RegidragoV` exclusion. These migrations remove two route-local physical-zone scans while keeping route admission, DCI/JIT timing, and discard strategy at their existing owners. Burnet route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_2408_burnet_resource_override.inc Surplus route: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_2271_surplus_regidrago_v_route_override.inc

Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136 DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next payload step: replace remaining ad hoc Dragon-payload membership and cardinality scans only where semantics exactly match `PayloadZonePolicy::contains()`, `contains_card()`, `contains_matching()`, `count()`, or `count_matching()`. Preserve physical-order selection when order is observable and preserve the explicit strategic order where preference is required. Keep DCI/JIT predicates and discard timing at strategy owners.

## Forretress cleanup

`src/trace_engine_v2/core/forretress/contract.inc` owns Engine member declarations and the card-facing board-role classifiers for Pineco, Forretress ex, the combined Pineco -> Forretress ex line, and the Regidrago V line. `src/trace_engine_v2/core/forretress/runtime.inc` owns runtime composition and route-facing definitions. `src/trace_engine_v2/core/forretress/exploding_energy_runtime.inc` owns the contiguous printed Exploding Energy resolver, Forretress-stack discard helper, board-index target adapter, and immediate post-KO promotion resolver. Promotion code reuses `is_regidrago_board_pokemon()` instead of maintaining a second Regidrago-family membership check. Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1 Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2 Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136

`src/trace_engine_v2/core/board_state_policy.inc` owns Active-first traversal, `BoardIndex` vocabulary, attachment-destination storage, pointer-to-index conversion, index lookup, exact-card source discovery, deterministic ranked board queries, and the prior-turn evolution timing predicate. Canonical board owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc

`src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc` reuses one local Bench existence traversal across its VSTAR and evolvable-Regidrago checks, and both Active and Bench evolution checks reuse `pokemon_can_evolve_into_regidrago_vstar()`. This removes a duplicated prior-turn Regidrago evolution predicate while keeping Burnet route admission and Supporter contention at the existing strategy owner. Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136 Board policy owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc Burnet route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/professor_burnet_ready_turn_policy.inc

Board-object Pineco identity in the evolution-timing queries and Regidrago-line identity in the Exploding Energy retreat path now reuse the contract classifiers. State-count queries still intentionally use exact `Card` counts, and retreat ranking still distinguishes Regidrago VSTAR from Regidrago V because that ordering is strategic rather than a board-family membership question.

Next mechanical Forretress step: inventory the remaining orchestration in `runtime.inc` and adjacent root route fragments for another complete semantic boundary, and replace only board-object identity or prior-turn evolution checks whose semantics exactly match existing board-policy classifiers. Preserve state-count queries, entry-turn evolution timing, route ordering, attachment distribution, retreat planning, and strategic ranking at their existing owners. Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117 Core evolution rules: https://www.pokemon.com/us/pokemon-tcg/rules Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085

## Steven route cleanup

Named Steven route policies live under `src/trace_engine_v2/core/routes/`. `src/trace_engine_v2/part_issue_1191_gladion_steven_override.inc` now owns the single historical Gladion/Steven/Treasure composition continuation at the parent `play_gladion` macro boundary. The former `part_issue_1204_gladion_treasure_override.inc` fragment has been retired; its unchanged continuation now follows `core/routes/gladion_steven_route_policy.inc` in that owner so the canonical route policy, direct rule/card sources, and the next Gladion layer remain adjacent. Parent alias lifetime remains owned by `composition/post_014a_overrides.inc`. Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145 Gladion: https://api.pokemontcg.io/v2/cards/sm4-95 Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

Projected Item-lock timing for the active-VSTAR Treasure continuation and late VSTAR/Vessel continuation now delegates to the canonical Engine `item_locked_on_turn()` seam instead of re-encoding lock-family identities inside route files. Shared timing owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

Next Steven cleanup step: inventory remaining root `part_*steven*` fragments and retire only composition-only forwarders whose canonical `core/routes/` owner can replace them at the identical textual boundary. Preserve route admission, DCI/UDP/AMR, Supporter contention, connector domination, hidden-information sequencing, and source URLs. Route-local projected lock checks should reuse `item_locked_on_turn()` whenever their semantics are exactly the shared persistent Item-lock schedule.

## Setup lifecycle cleanup

`src/trace_engine_v2/core/setup_lifecycle.inc` owns setup-facing deck/scenario labels together with opening-deck initialization, opening-hand and mulligan mechanics, Prize dealing, and setup-trace output. `src/trace_engine_v2/part_005.inc` composes that canonical owner at the established Engine member boundary.

`SetupRecipePolicy` owns setup recipe-presence and exact-count predicates used by deck/scenario classification. `prepare_opening_deck()` now directly owns knowledge reset, recipe population, and the opening shuffle after retiring its forwarding-only `reset_setup_knowledge()` and `add_recipe_cards_to_deck()` helpers. `draw_opening_hand_with_mulligans()` now directly owns each seven-card transfer, Basic check, return-and-reshuffle cycle, and mulligan count after retiring the one-use `draw_opening_hand_once()` helper. Scenario summaries call `SetupLifecycleConfig` directly instead of retaining forwarding-only label wrappers. `SetupLifecycleConfig::kUnknownLabel` centralizes the fallback label shared by DCI and lock rendering, keeping setup display vocabulary in one owner. Advanced setup procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Official rules: https://www.pokemon.com/us/pokemon-tcg/rules

Next setup step: route future setup recipe classification through `SetupRecipePolicy` and move only state-transition helpers from opening Active/Bench setup into `core/setup_lifecycle.inc` once exact source-contract coverage exists for hand removal, `started_regi`, Bench insertion, and declaration ordering. Keep strategic route predicates in Engine, and avoid reintroducing one-use forwarding helpers around lifecycle steps already expressed directly in the canonical owner.

## Catalog and knowledge cleanup

`src/trace_engine_v2/core/card_catalog.inc` now owns both the shrinking legacy name bridge and the intrinsic classification compatibility seam. `LegacyCardCatalog::name()` owns fallback-table traversal directly, while registered `CardDefinition` lookup remains canonical for migrated names and intrinsic metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp

The catalog names its structural recipe vocabulary once through `DeckRecipeEntry`, while `DeckRecipe` reuses that row type. The merged intrinsic predicates continue to delegate registered Pokémon stage/type, Retreat Cost, Rule Box, Pokémon V, Energy, Trainer subtype, and ACE SPEC facts to `CardDefinition`, retaining hard-coded compatibility only for unmigrated cards. Catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc

The global `name(Card)` compatibility seam falls directly from registered `CardDefinition` lookup to `LegacyCardCatalog::name()` without a forwarding-only legacy helper. Regidrago VSTAR has left the fallback table because its exact `CardDefinition` is registered. Keep future name cleanup on those two owners rather than adding another name-routing layer. Exact print: https://api.pokemontcg.io/v2/cards/swsh12-136

The former `src/trace_engine_v2/core/card_classification.inc` has been retired after moving its declarations unchanged beside the catalog lookup seam. Registered Pokémon Retreat Cost still resolves through `CardDefinition::retreat_cost`, and `is_basic()` plus broad `is_pokemon()` still share `is_legacy_basic_pokemon()` for unmigrated fallback identity. Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104 Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1

`src/trace_engine_v2/core/deck_knowledge.inc` keeps copy arithmetic behind `KnowledgeCopyPolicy`. `KnowledgeCopyPolicy::combined()` owns two-source aggregation, while `combined_public_zones()` owns hand/discard/attached arithmetic after Engine callers have resolved visibility. K1 hand/deck counts continue to reuse `combined()`. K0/K1 visibility rules remain unchanged at their Engine callers: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states

Next catalog/knowledge step: migrate remaining legacy name and intrinsic metadata rows only after their explicit `CardDefinition` is registered and covered. Move repeated copy-count arithmetic into `KnowledgeCopyPolicy` only when visibility has already been resolved by the Engine caller. Do not recreate a second classification fragment, forwarding-only catalog lookup members, or duplicate registered Retreat Cost or name metadata in compatibility switches. Hidden-zone visibility, Prize deduction, search timing, target preference, DCI/UDP/AMR, and route admission remain strategy concerns.

## Shared policy owners

- Dragon payload queries: `src/trace_engine_v2/core/payload_hand_policy.inc`.
- Garbodor scenario and Ability-lock composition: `src/trace_engine_v2/core/garbodor_lock_policy.inc`. `GarbodorScenarioPolicy::activation_turn_reached()` owns the shared turn threshold, and `GarbodorScenarioPolicy::active()` composes timing with scenario identity. `GarbodorAbilityLockPolicy::garbotoxin_locked()` owns present-versus-removed lock state, `rule_box_ability_available()` owns the separate Path-style Rule Box gate, `ability_available()` composes already-resolved lock facts, and `ability_available_from_lock_state()` centralizes the repeated four-input composition for Engine callers that already own those state facts. Engine wrappers remain compatibility/query seams for callers that need individual state-derived facts. Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
- Setup lifecycle labels, mulligans, Prize deal, and setup trace mechanics: `src/trace_engine_v2/core/setup_lifecycle.inc`.
- Recovery Supporter policy: `src/trace_engine_v2/core/recovery_supporter_policy.inc`.
- Turn action runtime: `src/trace_engine_v2/turn_action_policy_runtime.inc`.

Before adding a new loop or route-local helper, check these owners and reuse a named seam when ordering and semantics match exactly.

Next shared-policy step: route duplicate Garbodor present/removed plus Rule Box composition through `ability_available_from_lock_state()` only when all four state facts have the same semantics. Keep `garbotoxin_locked()`, `rule_box_ability_available()`, and `ability_available()` available for callers that already hold narrower or resolved facts. Preserve separate scenario identity, activation-turn, lock-removal, and Rule Box queries where callers need one fact without the others.

## Turn lifecycle cleanup

`src/trace_engine_v2/core/turn_lifecycle.inc` owns per-turn resets through two explicit policy owners. `TurnActionStatePolicy::reset()` clears generic action flags and same-turn discard tracking. `TransientTurnLockPolicy::reset()` owns the scenario-dependent one-turn Garbodor unlock reset. `reset_per_turn_state()` now composes both policies directly, without forwarding-only reset members. The established lifecycle order remains: set turn, clear action state, restore transient lock pressure, then perform the mandatory start-of-turn draw. Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104 Garbodor: https://api.pokemontcg.io/v2/cards/xy9-57 Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125 Advanced rules: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md Scenario contract: https://github.com/FlareZ123/pokemon-sims/issues/2808

Next turn-lifecycle step: route only exact duplicate action-flag/reset bundles through `TurnActionStatePolicy::reset()` and exact scenario-scoped transient lock resets through `TransientTurnLockPolicy::reset()`. Preserve the current ordering relative to the required turn draw, and keep persistent matchup state outside these per-turn policy owners.

## Projection cleanup

`src/trace_engine_v2/composition/post_014a_overrides.inc` gives the Tate public-projection recursion guard a named Engine member type instead of rebuilding an anonymous RAII type at each call site. The projection still isolates only Legacy Star and restores the same thread-local depth on scope exit: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc

`src/trace_engine_v2/part_roseanne_multimode_override.inc` now evaluates the Evolution Incense -> Earthen Vessel admission path on a copied `Engine`, matching the neighboring Pokémon Communication projection and avoiding temporary mutation/restoration of live hand state. Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148 Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163 Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163

Next projection step: migrate only admission checks whose semantics are already pure projections onto copied Engine state. Keep physical resolution, trace emission, K0/K1 transitions, and strategic route choice at their current owners.

## Validation gate

A cleanup PR is mergeable only when strict Release compilation succeeds, focused tests and the full regression suite show no new failure, sanitizer/structural checks show no new failure, representative `--simulate-this` traces preserve legal action ordering/readiness, the paired T2/T3 matrix has no unexplained drift, and the PR contains no gameplay behavior change.

Known baseline failures must be tied to their existing issue and shown unchanged. Any newly discovered gameplay defect uses the separate bug-confirmation workflow instead of combining the fix with cleanup.