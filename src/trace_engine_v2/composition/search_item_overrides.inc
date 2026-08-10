#pragma once

// Centralize the late search-item wrapper families behind one ordered boundary.
// The sequence is behavior-significant because each layer exposes the implementation
// consumed by the next override, including the three bridge layers still stored as
// legacy part files.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_oricorio_needed_energy_override.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "earthen_vessel_overrides.inc"
#include "search_route_overrides.inc"
