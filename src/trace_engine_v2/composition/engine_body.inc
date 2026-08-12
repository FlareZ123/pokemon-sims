#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core and late-reporting stages live in named composition units so the legacy
// forwarding wrappers are unnecessary. Preserve this textual include sequence and
// the macro lifetimes because several legacy parts deliberately cross boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core Engine state composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/core_state.inc
// Opening, search, Steven/Blender, Supporter, and VSTAR composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Late execution/reporting composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/late_execution_reporting.inc

#include "core_state.inc"

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
#include "../core/garbodor_lock_policy.inc"

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

// Compose the #3057 Pineco/Secret Box lock-removal wrapper directly at the
// post-search boundary instead of forwarding through a one-purpose .inc file.
// Forest / Field Blower / Secret Box: https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sm2-125 https://api.pokemontcg.io/v2/cards/sv6-163
// Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/3057
#include "../part_issue_3057_swell_field_blower_route.inc"
#if defined(play_chaotic_swell) || defined(play_field_blower)
#error "Issue-3057 Pineco composition requires lock-removal aliases to be undefined"
#endif
#define play_chaotic_swell play_chaotic_swell_issue_3057
#define play_field_blower play_field_blower_issue_3057
#include "post_014a_overrides.inc"
#undef play_field_blower
#undef play_chaotic_swell

#include "late_execution_reporting.inc"
