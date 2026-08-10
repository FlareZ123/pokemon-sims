#pragma once

// Compose the member overrides that become legal immediately after part_014a.inc
// closes run_search_items_one_step(). Keeping this boundary named avoids exposing
// independent override families in the translation-unit root while preserving
// their established dependency order.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
// Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179

#include "../part_issue_1683_serena_dead_quick_ball_override.inc"
#include "burnet_fss_overrides.inc"
#include "celestial_roar_overrides.inc"
#include "final_engine_overrides.inc"