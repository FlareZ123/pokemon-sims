#pragma once

// Centralize the late Earthen Vessel wrapper chain. Each alias exposes the
// preceding implementation to the next override, so include order is fixed.
// Earthen Vessel card text: https://api.pokemontcg.io/v2/cards/sv4-163
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#ifdef play_earthen_vessel
#error "Earthen Vessel composition requires play_earthen_vessel to be undefined on entry"
#endif

#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"

#ifdef play_earthen_vessel
#error "Earthen Vessel composition leaked play_earthen_vessel"
#endif
