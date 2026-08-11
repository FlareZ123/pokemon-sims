#pragma once

// Single ordered composition spine for the simulator Engine implementation.
// Core and late-reporting stages live here directly so the legacy forwarding
// wrappers are unnecessary. Preserve this textual include sequence and the macro
// lifetimes because several legacy parts deliberately cross member boundaries.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Core source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_002.inc
// Opening, search, Steven/Blender, Supporter, and VSTAR composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/opening_engine_overrides.inc
// Part 010 search/Steven source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010.inc
// Search-item closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014a.inc
// Post-search Supporter, Legacy Star, and late search-item composition: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/post_014a_overrides.inc
// Late turn source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc

// Core Engine state. part_002 owns the ordered catalog -> classification -> state chain.
#include "../part_002.inc"

#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"

// Route every later Pokemon-Ability legality check through the Garbotoxin-aware
// wrapper after part_003 defines the historical predicate.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
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

// Ordered late turn-execution, scenario-registry, and reporting composition.
// Keep these aliases live through part_016 exactly as the historical root wrapper
// did so the translation unit remains behaviorally identical.
#define play_field_blower play_field_blower_original
#define run_turn run_turn_original
#include "../part_014c.inc"
#undef ability_available_for_pokemon

// part_014c defines the historical scenario registry and leaves only the final
// return/brace of scenario_by_label() for the first two lines of part_015. The
// Garbodor wrapper is composed at namespace scope through part_forretress_ex_combo.inc.
// Route reporting and the production CLI through the appended two rows while the
// historical public registry remains available to focused tests.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Model specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
#define all_scenarios all_scenarios_with_garbodor
#define scenario_by_label scenario_by_label_with_garbodor
#include "../part_015.inc"

// play_field_blower, run_turn, and the scenario aliases intentionally remain live
// for part_016.inc.
// Translation-unit closure: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
#include "../part_016.inc"
#undef scenario_by_label
#undef all_scenarios
