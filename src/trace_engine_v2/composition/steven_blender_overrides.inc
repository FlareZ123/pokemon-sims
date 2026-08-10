#pragma once

// Centralize the core search, Steven, Forest Seal Stone, and Brilliant Blender
// wrapper stack. Macro entry/exit guards make this composition boundary explicit.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(play_evolution_incense) || defined(play_earthen_vessel) || \
    defined(play_brilliant_blender)
#error "Steven/Blender composition requires search aliases to be undefined on entry"
#endif

#define play_evolution_incense play_evolution_incense_original
#define play_earthen_vessel play_earthen_vessel_empty_deck_original
#define play_brilliant_blender play_brilliant_blender_legacy_original
#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#define should_play_steven should_play_steven_original
#include "../part_010.inc"
#undef should_play_steven
#undef attach_fss
#undef fss_target_after_search_started

#include "../part_010_late_steven_override.inc"

#define should_play_steven should_play_steven_issue1030_original
#include "../part_010_steven_crispin_override.inc"
#undef should_play_steven
#undef play_brilliant_blender

#include "steven_blender_thinning_overrides.inc"

// Continue into the Tapu and late search-item wrapper family at the same macro
// boundary previously represented by the next root include.
// Composition contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/tapu_search_overrides.inc
#include "tapu_search_overrides.inc"
