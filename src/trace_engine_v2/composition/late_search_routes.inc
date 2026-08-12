#pragma once

// Consolidated late-search route composition. Keep these units in this order because
// later wrappers consume member implementations exposed by the preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// Empty-deck search wrapper stack. Keep this order because later wrappers consume
// implementations exposed by the preceding layer. This section owns only
// preprocessor composition.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Historical wrapper: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/composition/empty_deck_search_routes.inc
#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Empty-deck search composition requires wrapper aliases to be undefined on entry"
#endif

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
#if defined(play_arven)
#error "Arven wrapper alias must be released before the Garbodor continuation"
#endif
#include "../part_arven_garbodor_field_blower_override.inc"

#undef use_fss
#undef play_steven
#if defined(use_fss) || defined(play_steven)
#error "FSS and Steven aliases must be released before the FSS-energy continuation"
#endif
#include "../part_issue_1356_fss_energy_override.inc"

#undef play_quick_ball
#undef play_mysterious_treasure
#undef in_play
#undef bench_tapu_if_useful

#if defined(play_steven) || defined(should_play_steven) || \
    defined(bench_tapu_if_useful)
#error "Late empty-deck selector aliases must be undefined before reuse"
#endif

#define play_steven play_steven_issue1030_original
#include "../part_issue_1002_steven_target_override.inc"
#undef play_steven

#define should_play_steven should_play_steven_issue1067_original
#include "../part_issue_1030_steven_turo_override.inc"
#undef should_play_steven

#define should_play_steven should_play_steven_issue3197_original
#define TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS play_steven_issue3197_original
#include "../part_issue_1067_arven_before_late_steven_override.inc"
#undef TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS
#undef play_steven
#undef should_play_steven
#include "../part_issue_3197_steven_treasure_crispin_semantic_override.inc"

#define bench_tapu_if_useful bench_tapu_if_useful_issue_991_original
#include "../part_issue_989_empty_deck_tapu_override.inc"
#undef bench_tapu_if_useful

#include "../part_issue_991_wonder_tag_burnet_legacy_star_override.inc"

#undef play_earthen_vessel
#undef play_brilliant_blender
#undef play_ultra_ball
#undef ultra_ball_has_legal_target
#undef bench_oricorio_if_useful

#if defined(bench_oricorio_if_useful) || defined(ultra_ball_has_legal_target) || \
    defined(play_ultra_ball) || defined(play_brilliant_blender) || \
    defined(play_earthen_vessel) || defined(play_mysterious_treasure) || \
    defined(play_quick_ball) || defined(bench_tapu_if_useful) || \
    defined(in_play) || defined(play_steven) || defined(use_fss) || \
    defined(should_play_steven) || defined(play_arven)
#error "Empty-deck search composition leaked a wrapper alias"
#endif

#include "../core/ultra_ball_target_policy.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "../core/oricorio_connector_policy.inc"

#include "post_search_connector_routes.inc"
#include "quick_ball_tail.inc"
