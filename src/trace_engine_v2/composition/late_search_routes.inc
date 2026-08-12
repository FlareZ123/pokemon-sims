#pragma once

// Consolidated late-search route composition. Keep the historical layers in this
// order because later wrappers consume member implementations exposed by the
// preceding layer. This file only centralizes preprocessor composition; route logic
// remains in the referenced implementation files.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "empty_deck_search_stack.inc"

// Post-search Earthen Vessel and Mysterious Treasure connector stack.
#if defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(choose_supporter) || defined(in_play)
#error "Post-search connector composition requires aliases to be undefined on entry"
#endif

#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"

#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "../part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure

#define play_quick_ball play_quick_ball_issue1235_original
#define play_mysterious_treasure play_mysterious_treasure_issue1235_original
#define choose_supporter choose_supporter_issue1235_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_issue_1209_treasure_tapu_crispin_override.inc"
#undef in_play
#undef choose_supporter
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1356_original
#define play_mysterious_treasure play_mysterious_treasure_issue1356_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_issue_1235_oricorio_treasure_tapu_override.inc"
#undef in_play
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1476_original
#define play_mysterious_treasure play_mysterious_treasure_issue1476_original
#include "../part_issue_1356_fss_treasure_energy_override.inc"
#undef play_mysterious_treasure
#undef play_quick_ball

#if defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(choose_supporter) || defined(in_play)
#error "Post-search connector composition leaked a wrapper alias"
#endif

// Quick Ball tail.
#if defined(play_quick_ball)
#error "Quick Ball route composition requires play_quick_ball to be undefined on entry"
#endif

#define play_quick_ball play_quick_ball_issue1516_original
#include "../part_issue_1476_redundant_burnet_route_override.inc"
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1595_original
#include "../part_issue_1516_quick_ball_tapu_crispin_override.inc"
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue2272_original
#include "../part_issue_1595_preserve_turo_over_quick_ball_override.inc"
#undef play_quick_ball

#include "../part_issue_2272_route_replaced_arven_quick_ball_override.inc"

#if defined(play_quick_ball)
#error "Quick Ball route composition leaked play_quick_ball"
#endif
