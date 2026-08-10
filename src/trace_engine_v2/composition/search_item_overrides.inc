#pragma once

// Centralize the late search-item wrapper families behind one ordered boundary.
// The sequence is behavior-significant because each layer exposes the implementation
// consumed by the next override, including the compatibility bridge layer still
// backed by legacy part files.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss)
#error "Late search composition requires search wrapper aliases to be undefined on entry"
#endif

#include "empty_deck_search_overrides.inc"
#include "search_item_legacy_bridges.inc"
#include "earthen_vessel_overrides.inc"
#include "treasure_route_overrides.inc"
#include "quick_ball_route_overrides.inc"

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss)
#error "Late search composition leaked a wrapper alias"
#endif
