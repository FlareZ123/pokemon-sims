#pragma once

// Compose the Quick Ball-only tail of the late search wrapper chain. Each alias
// deliberately exposes the immediately preceding implementation to the next layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#ifdef play_quick_ball
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

#ifdef play_quick_ball
#error "Quick Ball route composition leaked play_quick_ball"
#endif
