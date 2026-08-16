#pragma once

// Canonical namespace-scope scenario-registry owner for the Forretress package.
// Keep public registry declarations beside the extensions that consume them so
// package.inc only owns ordered feature composition.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

// Reuse the canonical append/lookup abstraction and keep the concrete Garbodor
// scenario family adjacent to its registry contract.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
#include "scenario_extension.inc"
#include "garbodor_scenario_extension.inc"
