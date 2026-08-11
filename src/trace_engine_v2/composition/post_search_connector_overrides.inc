#pragma once

// Ordered post-search connector composition for Earthen Vessel and Mysterious Treasure.
// This first consolidation preserves the existing per-family entry/leak guards and exact
// alias/include order before the compatibility wrappers are redirected here.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

// Earthen Vessel card text: https://api.pokemontcg.io/v2/cards/sv4-163
#ifdef play_earthen_vessel
#error "Earthen Vessel composition requires play_earthen_vessel to be undefined on entry"
#endif

#define play_earthen_vessel play_earthen_vessel_issue1368_original
#include "../part_earthen_vessel_vstar_window_override.inc"
#undef play_earthen_vessel

#define play_earthen_vessel play_earthen_vessel_issue1412_original
#include "../part_issue_1368_earthen_vessel_celestial_roar_override.inc"
#undef play_earthen_vessel

#include "../part_issue_1412_preserve_quick_balls.inc"

#ifdef play_earthen_vessel
#error "Earthen Vessel composition leaked play_earthen_vessel"
#endif

// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Copy-aware K1 connector regression: https://github.com/FlareZ123/pokemon-sims/issues/3129
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
#if defined(play_mysterious_treasure) || defined(play_quick_ball) || \
    defined(choose_supporter) || defined(in_play)
#error "Treasure route composition requires wrapper aliases to be undefined on entry"
#endif

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

#if defined(play_mysterious_treasure) || defined(play_quick_ball) || \
    defined(choose_supporter) || defined(in_play)
#error "Treasure route composition leaked a wrapper alias"
#endif
