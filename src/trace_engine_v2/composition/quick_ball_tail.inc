#pragma once

// Centralized Quick Ball tail composition. Keep the historical wrappers ordered so
// each layer consumes the member implementation exposed immediately before it.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Quick Ball card data: https://api.pokemontcg.io/v2/cards/swsh1-179
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

// Keep the historical final wrapper compiled under a dormant name so its route
// admission helpers remain available. The active final member is supplied by the
// card-class bridge immediately afterward.
#define play_quick_ball play_quick_ball_card_class_tail_legacy
#include "../part_issue_2272_route_replaced_arven_quick_ball_override.inc"
#undef play_quick_ball
#include "../core/quick_ball_card_class_tail.inc"

#if defined(play_quick_ball)
#error "Quick Ball route composition leaked play_quick_ball"
#endif
