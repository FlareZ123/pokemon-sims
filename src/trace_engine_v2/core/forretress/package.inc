#pragma once

// Canonical namespace-scope composition package for the Forretress feature.
// Runtime and scenario-extension ownership stay together under core/forretress
// while Engine is closed, preserving the historical declaration and include order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "runtime.inc"

// The historical registry is defined later in part_014c.inc. Keep its public
// declarations behind one named contract at this verified namespace boundary:
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
// Canonical contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/scenario_contract.inc
#include "scenario_contract.inc"

// Reuse the canonical append/lookup abstraction and keep the concrete Garbodor
// scenario family beside that shared Forretress-side machinery.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
#include "scenario_extension.inc"
#include "garbodor_scenario_extension.inc"