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
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"
#include "post_014a_overrides.inc"

// #3057/#3064 preserves the Stadium action for a live current-turn Forest/Pineco/
// Forretress route. The helper also owns the deterministic public-discard Forest
// recovery used immediately after Field Blower removes Path:
// Forest / Field Blower / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh12-136
// Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
// Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
#include "../part_issue_3057_swell_field_blower_route.inc"

// Ordered late turn-execution, scenario-registry, and reporting composition.
// The historical turn calls advance_forretress_combo() immediately after its Path
// removal attempts. Scope the #3064 wrapper to this late source so the public Forest
// recovery runs at that exact transition while every nonmatching state delegates to
// the original planner:
// Late execution order: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc
// Forest / Field Blower / Legacy Star: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/swsh12-136
// Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(advance_forretress_combo)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057
#define advance_forretress_combo advance_forretress_combo_issue3064
#include "../part_014c.inc"
#undef advance_forretress_combo
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
#undef scenario_by_label
#undef all_scenarios
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition leaked an alias"
#endif
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count) || \
    defined(advance_forretress_combo)
#error "Late-turn composition leaked an execution alias"
#endif
