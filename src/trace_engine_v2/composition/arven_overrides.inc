#pragma once

// Keep the historical Arven override stack in one ordered composition unit.
// Each wrapper aliases the implementation immediately before it, so preserve
// both include order and the matching undef boundary exactly.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#define play_arven play_arven_powerglass_original
#include "../part_012_override.inc"
#undef play_arven
#define play_arven play_arven_fss_blender_contention_original
#include "../part_012_powerglass_override.inc"
#undef play_arven
#define play_arven play_arven_empty_deck_original
#include "../part_012_arven_fss_blender_contention_override.inc"
#undef play_arven
