#pragma once

// Centralize the core search, Steven, Forest Seal Stone, and Brilliant Blender
// wrapper stack. Macro entry/exit guards make this composition boundary explicit.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#ifdef play_evolution_incense
#error "Steven/Blender composition requires play_evolution_incense to be undefined on entry"
#endif
#ifdef play_earthen_vessel
#error "Steven/Blender composition requires play_earthen_vessel to be undefined on entry"
#endif
#ifdef play_brilliant_blender
#error "Steven/Blender composition requires play_brilliant_blender to be undefined on entry"
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

// The thinning policy remains the implementation wrapped by the later empty-deck
// guard, while the legacy part_010 implementation stays dormant:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_blender_thinning_override.inc#L1-L77
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_empty_deck_search_override.inc#L78-L85
#define play_brilliant_blender play_brilliant_blender_empty_deck_original
#include "../part_010_blender_thinning_override.inc"
#undef play_brilliant_blender
#undef play_earthen_vessel
#undef play_evolution_incense
#undef play_ultra_ball

#ifdef play_evolution_incense
#error "Steven/Blender composition leaked play_evolution_incense"
#endif
#ifdef play_earthen_vessel
#error "Steven/Blender composition leaked play_earthen_vessel"
#endif
#ifdef play_brilliant_blender
#error "Steven/Blender composition leaked play_brilliant_blender"
#endif
