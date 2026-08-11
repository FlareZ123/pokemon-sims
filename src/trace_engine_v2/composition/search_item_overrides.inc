#pragma once

// Centralize the late search-item wrapper families behind one ordered boundary.
// The sequence is behavior-significant because each layer exposes the implementation
// consumed by the next override, including legacy fragments that are still composed
// directly at this boundary.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Late search composition requires search wrapper aliases to be undefined on entry"
#endif

// Empty-deck search family. Macro aliases deliberately expose the preceding
// implementation to later wrappers, so preserve both include order and each
// matching undef boundary.
#define bench_oricorio_if_useful bench_oricorio_if_useful_target_original
#define ultra_ball_has_legal_target ultra_ball_has_legal_target_k0_target_original
#define play_ultra_ball play_ultra_ball_k0_target_original
#define play_brilliant_blender play_brilliant_blender_vstar_axis_original
#define play_earthen_vessel play_earthen_vessel_vstar_window_original
#define play_mysterious_treasure play_mysterious_treasure_issue1167_original
#define play_quick_ball play_quick_ball_issue1209_original
#define bench_tapu_if_useful bench_tapu_if_useful_issue989_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#define play_steven play_steven_issue1002_empty_deck_original
#define use_fss use_fss_issue1356_original
#define play_arven play_arven_garbodor_original
#include "../part_empty_deck_search_override.inc"

#undef play_arven
#include "../part_arven_garbodor_field_blower_override.inc"

#undef use_fss
#undef play_steven
#include "../part_issue_1356_fss_energy_override.inc"

#undef play_quick_ball
#undef play_mysterious_treasure
#undef in_play
#undef bench_tapu_if_useful

#define play_steven play_steven_issue1030_original
#include "../part_issue_1002_steven_target_override.inc"
#undef play_steven

#define should_play_steven should_play_steven_issue1067_original
#include "../part_issue_1030_steven_turo_override.inc"
#undef should_play_steven

#include "../part_issue_1067_arven_before_late_steven_override.inc"

#define bench_tapu_if_useful bench_tapu_if_useful_issue_991_original
#include "../part_issue_989_empty_deck_tapu_override.inc"
#undef bench_tapu_if_useful

#include "../part_issue_991_wonder_tag_burnet_legacy_star_override.inc"

#undef play_earthen_vessel
#undef play_brilliant_blender
#undef play_ultra_ball
#undef ultra_ball_has_legal_target
#undef bench_oricorio_if_useful

// Legacy state/target bridges remain direct composition inputs until each fragment
// is migrated into its owning card-action family.
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_blender_vstar_axis_override.inc"

// Core connector policies are composed directly without compatibility wrappers.
#include "../core/oricorio_connector_policy.inc" // Oricorio Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55

// Card-action route families own the later search-item overrides.
#include "earthen_vessel_overrides.inc" // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
#include "treasure_route_overrides.inc" // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
#include "quick_ball_route_overrides.inc" // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Late search composition leaked a wrapper alias"
#endif
