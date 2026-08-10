#pragma once

// Ordered composition for the core Engine state, turn, and trace body. This keeps
// the historical split parts in one named boundary while preserving the macro
// lifetime consumed by opening_engine_overrides.inc.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Composition root: https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp

#include "../part_000.inc"
#include "../part_001.inc"
#include "../part_002.inc"

#define begin_turn begin_turn_original
#define might_be_unseen might_be_unseen_empty_deck_original
#include "../part_003.inc"
// Route every later Pokemon-Ability legality check through the Garbotoxin-aware
// wrapper. part_003 contains the base predicate definition, so defining the alias
// only after that include preserves the original implementation for delegation:
// https://api.pokemontcg.io/v2/cards/xy9-57
// https://github.com/FlareZ123/pokemon-sims/issues/2808
#define ability_available_for_pokemon ability_available_for_pokemon_garbodor

// part_003.inc opens begin_turn(), part_004.inc completes it and later opens
// state_line(), and part_005.inc closes state_line() before the first override
// member is introduced:
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