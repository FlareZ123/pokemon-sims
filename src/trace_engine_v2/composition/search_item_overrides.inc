#pragma once

// Centralize the late search-item wrapper families behind one ordered boundary.
// The sequence is behavior-significant because each layer exposes the implementation
// consumed by the next override, including the three bridge layers still stored as
// legacy part files.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss)
#error "Late search composition requires search wrapper aliases to be undefined on entry"
#endif

#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_oricorio_needed_energy_override.inc"
#include "../part_blender_vstar_axis_override.inc"

// Earthen Vessel wrapper chain. Each alias exposes the preceding implementation
// to the next override, so the define/include/undef order is behavior-significant.
#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"
#include "search_route_overrides.inc"

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss)
#error "Late search composition leaked a wrapper alias"
#endif
