#pragma once

// Arven wrappers on the post-search composition boundary. Each alias exposes
// the implementation from the preceding layer without leaking macro state.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
#ifdef play_arven
#error "Arven composition requires play_arven to be undefined on entry"
#endif

#define play_arven play_arven_powerglass_original
#include "../part_012_override.inc"
#undef play_arven

#define play_arven play_arven_fss_blender_contention_original
#include "../part_012_powerglass_override.inc"
#undef play_arven

#define play_arven play_arven_empty_deck_original
#include "../part_012_arven_fss_blender_contention_override.inc"
#undef play_arven

#ifdef play_arven
#error "Arven composition leaked play_arven"
#endif
