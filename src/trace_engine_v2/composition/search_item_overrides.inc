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

// Base search policy first, then the compatibility aliases that adapt legacy
// fragments to the current composition boundary. Their order is intentional.
#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_oricorio_needed_energy_override.inc"
#include "../part_blender_vstar_axis_override.inc"

// Item-specific route families stay grouped by the card action they override.
#include "earthen_vessel_overrides.inc" // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
#include "treasure_route_overrides.inc" // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
#include "quick_ball_route_overrides.inc" // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss)
#error "Late search composition leaked a wrapper alias"
#endif