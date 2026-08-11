#pragma once

// Ordered late-search composition consolidates the former empty-deck, connector,
// and Quick Ball wrapper files behind one member-boundary include. Keep this
// sequence stable because each wrapper consumes the implementation exposed by the
// preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Copy-aware K1 connector regression: https://github.com/FlareZ123/pokemon-sims/issues/3129

#include "late_search_empty_deck.inc"
#include "late_search_connectors.inc"

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
