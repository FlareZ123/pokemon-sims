#pragma once

// Ordered late-search composition is centralized behind one member-boundary include.
// Keep its internal layer order stable because each wrapper consumes the implementation
// exposed by the preceding layer.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
// Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
// Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Copy-aware K1 connector regression: https://github.com/FlareZ123/pokemon-sims/issues/3129

#include "late_search_routes.inc"
