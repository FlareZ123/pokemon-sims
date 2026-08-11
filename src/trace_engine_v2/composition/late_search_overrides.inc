#pragma once

// Centralized late-search composition boundary. Keep connector wrappers before the
// Quick Ball tail because each layer intentionally exposes the immediately preceding
// implementation through scoped preprocessor aliases.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55

// Ordered late-search connector stack formerly isolated in
// late_search_connector_stack.inc. Preserve this exact include order.
#include "empty_deck_search_overrides.inc"
#include "../part_k0_ultra_ball_target_override.inc"
#include "../part_blender_vstar_axis_override.inc"
#include "../core/oricorio_connector_policy.inc"
#include "post_search_connector_overrides.inc"

#include "late_quick_ball_overrides.inc"
