#pragma once

// Compose the Mysterious Treasure-led late search wrappers in one ordered unit.
// Each alias exposes the preceding implementation to the next route override, so
// the include order is behavior-significant.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#ifdef play_mysterious_treasure
#error "Treasure route composition requires play_mysterious_treasure to be undefined on entry"
#endif

#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "../part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure

#define play_quick_ball play_quick_ball_issue1235_original
#define play_mysterious_treasure play_mysterious_treasure_issue1235_original
#define choose_supporter choose_supporter_issue1235_original
#include "../part_issue_1209_treasure_tapu_crispin_override.inc"
#undef choose_supporter
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1356_original
#define play_mysterious_treasure play_mysterious_treasure_issue1356_original
#include "../part_issue_1235_oricorio_treasure_tapu_override.inc"
#undef play_mysterious_treasure
#undef play_quick_ball

#define play_quick_ball play_quick_ball_issue1476_original
#define play_mysterious_treasure play_mysterious_treasure_issue1476_original
#include "../part_issue_1356_fss_treasure_energy_override.inc"
#undef play_mysterious_treasure
#undef play_quick_ball

#ifdef play_mysterious_treasure
#error "Treasure route composition leaked play_mysterious_treasure"
#endif
