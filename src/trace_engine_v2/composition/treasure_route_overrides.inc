#pragma once

// Compose the Mysterious Treasure-led late search wrappers in one ordered unit.
// Each alias exposes the preceding implementation to the next route override, so
// the include order is behavior-significant.
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Copy-aware K1 connector regression: https://github.com/FlareZ123/pokemon-sims/issues/3129
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

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
