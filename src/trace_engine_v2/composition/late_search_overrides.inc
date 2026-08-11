#pragma once

// Centralized late-search composition boundary. Keep connector wrappers before the
// Quick Ball tail because each layer intentionally exposes the immediately preceding
// implementation through scoped preprocessor aliases.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
// Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113

#include "late_search_connector_stack.inc"
#include "late_quick_ball_overrides.inc"
