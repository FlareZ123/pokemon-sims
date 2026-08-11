#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core and late-reporting stages live here directly so the legacy forwarding
// wrappers are unnecessary. Preserve this textual include sequence and the macro
// lifetimes because several legacy parts deliberately cross member boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Card catalog: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Card classification: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
// Simulator state: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulator_state.inc
// Opening, search, Steven/Blender, Supporter, and VSTAR composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Part 010 search/Steven source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search Supporter, Legacy Star, and late search-item composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

// Keep the legacy include guards while composing the three core fragments directly.
// This removes one forwarding .inc layer without changing textual include order.
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

#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

// Route every later Pokemon-Ability legality check through the Garbotoxin-aware
// wrapper after part_003 defines the historical predicate.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#if defined(ability_available_for_pokemon)
#error "Garbodor composition requires ability_available_for_pokemon to be undefined on entry"
#endif
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor

// part_003.inc opens begin_turn(), part_004.inc completes it and later opens
// state_line(), and part_005.inc closes state_line() before the first override
// member is introduced.
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L151-L154
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L1-L22
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_005.inc#L1-L5
#include "../part_004.inc"
#include "../part_005.inc"
#include "../part_garbodor_ability_lock_override.inc"

#undef might_be_unseen
// begin_turn intentionally remains aliased until opening_engine_overrides.inc
// composes part_empty_deck_unseen_override.inc and releases the alias.
// Replace only the historical banked-Tapu implementation while preserving the
// rest of the opening override ordering. The replacement uses the shared DDE-aware
// Apex payment predicate: https://github.com/FlareZ123/pokemon-sims/issues/3089
#define retreat_banked_tapu_to_regidrago retreat_banked_tapu_to_regidrago_issue3089_original
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#undef retreat_banked_tapu_to_regidrago
// part_014a closes run_search_items_one_step(), so the completed replacement member
// is composed directly from core at the same safe class-member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/banked_tapu_retreat_policy.inc
// https://github.com/FlareZ123/pokemon-sims/issues/3089
#include "../core/banked_tapu_retreat_policy.inc"
#include "post_014a_overrides.inc"

// #3057 arbitrates the Path-removal connector and restores the Forretress fixed
// point after an Item search exposes Forest of Vitality. Keep both hooks scoped to
// the historical late-turn source so unrelated member implementations remain on
// their established paths:
// Forest / Field Blower / Secret Box: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sv6-163
// Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
#include "../part_issue_3057_swell_field_blower_route.inc"

// Ordered late turn-execution, scenario-registry, and reporting composition.
// Keep these aliases live through part_016 exactly as the historical root wrapper
// did so the translation unit remains behaviorally identical.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
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
#ifdef ability_available_for_pokemon
#error "Garbodor composition leaked ability_available_for_pokemon"
#endif

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. The
// Garbodor wrapper is composed at namespace scope through part_forretress_ex_combo.inc.
// Route reporting and the production CLI through the appended two rows while the
// historical public registry remains available to focused tests.
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

// Release the late-turn textual-composition aliases after part_016 closes the
// historical execution/reporting chain. Keep the leak guards beside the undefs.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#undef scenario_by_label
#undef all_scenarios
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition leaked an alias"
#endif
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(play_items_until_stable)
#error "Late-turn composition leaked an execution alias"
#endif
