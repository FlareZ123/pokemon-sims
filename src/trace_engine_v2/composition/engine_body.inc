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
// Post-search composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

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

// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#if defined(ability_available_for_pokemon)
#error "Garbodor composition requires ability_available_for_pokemon to be undefined on entry"
#endif
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor
#include "../part_004.inc"
#include "../part_005.inc"
#include "../part_garbodor_ability_lock_override.inc"

#undef might_be_unseen
#include "opening_engine_overrides.inc"
#include "../part_014a.inc"

// The historical late run_turn calls base use_legacy_star() directly. Rename that
// implementation while post-search composition defines it, then let #3064 expose
// the real member name as a narrow outer wrapper. This leaves all unrelated Legacy
// Star behavior on the historical implementation:
// Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
// Governing refinement: https://github.com/FlareZ123/pokemon-sims/issues/3057#issuecomment-5252233081
// Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
#if defined(use_legacy_star)
#error "Legacy Star wrapper alias must be undefined before post-search composition"
#endif
#define use_legacy_star use_legacy_star_before3064
#include "post_014a_overrides.inc"
#undef use_legacy_star

// Forest / Path / Field Blower / Chaotic Swell: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/swsh6-148 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sm12-187
// Corrective issue: https://github.com/FlareZ123/pokemon-sims/issues/3064
#include "../part_issue_3057_swell_field_blower_route.inc"

#if defined(play_field_blower) || defined(run_turn) || defined(hand_count)
#error "Late-turn composition requires execution aliases to be undefined on entry"
#endif
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#define hand_count hand_count_issue_3057
#include "../part_014c.inc"
#undef hand_count
#undef ability_available_for_pokemon
#ifdef ability_available_for_pokemon
#error "Garbodor composition leaked ability_available_for_pokemon"
#endif

// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition requires aliases to be undefined on entry"
#endif
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"
#include "../part_016.inc"
#undef scenario_by_label
#undef all_scenarios
#if defined(all_scenarios) || defined(scenario_by_label)
#error "Scenario-registry composition leaked an alias"
#endif
#undef run_turn
#undef play_field_blower
#if defined(play_field_blower) || defined(run_turn) || defined(hand_count)
#error "Late-turn composition leaked an execution alias"
#endif
