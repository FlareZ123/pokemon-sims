#pragma once

// Isolate the active Brilliant Blender thinning layer and release the search-item
// aliases at the same legacy boundary used by the parent Steven composition.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

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
