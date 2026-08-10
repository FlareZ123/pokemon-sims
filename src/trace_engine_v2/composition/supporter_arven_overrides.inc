#pragma once

// Arven wrapper stack. Keep the override chain isolated so the root Supporter
// composition exposes one named boundary instead of repeated macro choreography.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
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
