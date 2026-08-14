#pragma once

// Canonical ordered composition entry point for the simulator Engine implementation.
// Preserve this textual include sequence because legacy aliases deliberately cross
// stage boundaries before their owning sections release them.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Lock-removal policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forest_field_blower_policy.inc
// Lock-removal specification: https://github.com/FlareZ123/pokemon-sims/issues/3057
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc

#ifndef SIM_TRACE_SIMULATOR_STATE_INCLUDED
#define SIM_TRACE_SIMULATOR_STATE_INCLUDED

#ifndef SIM_TRACE_CARD_CATALOG_INCLUDED
#define SIM_TRACE_CARD_CATALOG_INCLUDED
#include "../core/card_catalog.inc"
#endif

#ifndef SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#define SIM_TRACE_CARD_CLASSIFICATION_INCLUDED
#include "../core/card_classification.inc"
#endif

#include "../core/simulator_state.inc"

#endif  // SIM_TRACE_SIMULATOR_STATE_INCLUDED

// Opening stage. Keep each phase's aliases local while preserving the historical
// textual include order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// Phase 1: open the legacy turn body and expose the Garbotoxin-aware Ability alias.
#if defined(begin_turn) || defined(might_be_unseen)
#error "Opening composition requires begin_turn and might_be_unseen aliases to be undefined on entry"
#endif
#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

#if defined(ability_available_for_pokemon)
#error "Garbodor composition requires ability_available_for_pokemon to be undefined on entry"
#endif
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor

// Phase 2: complete the legacy member continuation before opening overrides.
// part_003.inc opens begin_turn(), part_004.inc completes it and later opens
// state_line(), and part_005.inc closes state_line() before the first override
// member is introduced.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L1-L22
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc#L1-L5
#include "../part_004.inc"
#include "../part_005.inc"
#include "../core/garbodor_lock_policy.inc"

#undef might_be_unseen
#if defined(might_be_unseen)
#error "Opening composition leaked the temporary might_be_unseen alias"
#endif

// Phase 3: compose the banked-Tapu replacement at the post-search member boundary.
// Preserve this order because part_014a closes run_search_items_one_step() before
// the completed replacement policy is composed at the safe class-member boundary.
// DDE-aware Apex payment / confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/3089
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Replacement policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/banked_tapu_retreat_policy.inc
#if defined(retreat_banked_tapu_to_regidrago)
#error "Banked-Tapu composition requires the retreat alias to be undefined on entry"
#endif
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago
#if defined(retreat_banked_tapu_to_regidrago)
#error "Banked-Tapu composition leaked the temporary retreat alias"
#endif
#if defined(begin_turn)
#error "Banked-Tapu composition expected opening_engine_overrides.inc to release begin_turn"
#endif
#include "../core/routes/banked_tapu_retreat_policy.inc"

// Load the shared forest / Field Blower policy before exporting the two aliases
// required by post-search composition.
// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
// Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
// Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
#include "../core/forest_field_blower_policy.inc"

#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition requires lock-removal aliases to be undefined"
#endif
#define play_chaotic_swell play_chaotic_swell_issue_3057
#define play_field_blower play_field_blower_issue_3057
#include "post_014a_overrides.inc"
#undef play_field_blower
#undef play_chaotic_swell

#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition leaked a temporary lock-removal alias"
#endif

// Ordered late turn-execution, scenario-registry, and reporting composition is kept
// directly on the canonical body now that it has a single owner. Preserve these
// aliases through part_016 exactly as the historical wrapper did.
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057
#define play_items_until_stable play_items_until_stable_issue_3057
#include "../part_014c.inc"
#undef play_items_until_stable
#undef hand_count
#undef ability_available_for_pokemon

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. The
// Garbodor wrapper is composed at namespace scope through part_forretress_ex_combo.inc.
// Route reporting and the production CLI continue through the appended two rows while
// the historical public registry remains available to focused tests.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition requires aliases to be undefined on entry"
#endif
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for part_016.inc.
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
#include "../part_016.inc"

// Release every alias owned by this composition unit in one place after part_016
// closes the historical execution/reporting chain. Entry guards above validate the
// ownership boundary before these aliases are introduced.
#undef scenario_by_label
#undef all_scenarios
#undef run_turn
#undef play_field_blower

#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable) || defined(all_scenarios) || \
    defined(scenario_by_label)
#error "Canonical engine composition leaked a late alias"
#endif
