#pragma once

// Consolidated late-search route composition. Keep the historical layers in this
// order because later wrappers consume member implementations exposed by the
// preceding layer. This file only centralizes preprocessor composition; route logic
// remains in the referenced implementation files.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#include "empty_deck_search_stack.inc"
#include "post_search_connector_stack.inc"

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
