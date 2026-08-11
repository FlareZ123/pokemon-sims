#pragma once

// Ordered late-search connector stack extracted from post_014a_overrides.inc.
// Keep this include sequence stable because each wrapper builds on the implementation
// exposed by the preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55

#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "../core/oricorio_connector_policy.inc"
#include "post_search_connector_overrides.inc"
