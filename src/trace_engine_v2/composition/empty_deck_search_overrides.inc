#pragma once

// Keep the empty-deck search wrapper stack in one ordered composition unit.
// Macro aliases deliberately expose the preceding implementation to later wrappers,
// so preserve both include order and each matching undef boundary.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
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
#include "../part_empty_deck_search_override.inc"
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
