// Brilliant Blender thinning composition owner. Keep this include after the
// Steven/VSTAR Vessel layer has released the legacy Blender alias. The thinning
// implementation remains wrapped by the later empty-deck guard, then releases the
// search aliases at the same historical boundary.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Thinning policy source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_010_blender_thinning_override.inc#L1-L77
// Empty-deck wrapper source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_empty_deck_search_override.inc#L78-L85

#define play_brilliant_blender play_brilliant_blender_empty_deck_original
#include "../part_010_blender_thinning_override.inc"
#undef play_brilliant_blender
#undef play_earthen_vessel
#undef play_evolution_incense
#undef play_ultra_ball
