#pragma once

// Keep the Earthen Vessel wrapper sequence together so the root translation unit
// only names the composition boundary. The alias chain is behavior-significant.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"
