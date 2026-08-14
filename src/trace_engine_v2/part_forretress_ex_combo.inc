// Canonical namespace-scope composition boundary for the Forretress feature.
// Keep the runtime and scenario extension adjacent here while Engine is closed.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "core/forretress/runtime.inc"

// Garbodor scenario extension stays at this same verified namespace boundary.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
#include "core/forretress/garbodor_scenarios.inc"
