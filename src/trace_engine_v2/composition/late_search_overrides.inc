#pragma once

// Ordered late-search connector and Quick Ball stacks consolidated from the former
// late_search_connector_stack.inc and late_quick_ball_overrides.inc wrappers.
// Keep this sequence stable because every wrapper consumes the implementation
// exposed by the preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179

#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "../core/oricorio_connector_policy.inc"
#include "post_search_connector_overrides.inc"

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
