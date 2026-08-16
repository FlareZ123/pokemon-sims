// Base Steven search composition owner. The parent includes this file at the
// historical post-part_009b2 boundary while play_ultra_ball is intentionally live.
// Preserve this exact define/include/undef order because part_010 and the late
// Steven layer consume the aliases as class-member composition.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
// Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
// Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
// Canonical advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md

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
