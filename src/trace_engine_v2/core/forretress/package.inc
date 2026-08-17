#pragma once

// Canonical namespace-scope composition package for the Forretress feature.
// Runtime and scenario-extension ownership stay together under core/forretress
// while Engine is closed, preserving the historical declaration and include order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "runtime.inc"

// Public namespace-scope contract required by the Forretress scenario extensions.
// Keep these declarations at the package boundary before scenario_extension.inc so
// the package owns its complete namespace-scope composition without another
// forwarding fragment. C++ declaration rules: https://eel.is/c++draft/dcl.pre
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

// Reuse the canonical append/lookup abstraction and keep the concrete Garbodor
// scenario family in that same named owner.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
#include "scenario_extension.inc"
