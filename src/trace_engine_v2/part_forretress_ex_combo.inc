#pragma once

// Namespace-scope composition boundary for the Forretress feature.
// Keep runtime and scenario extension ownership under core/forretress while Engine
// is closed, preserving the historical declaration and include order here.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "core/forretress/runtime.inc"

// The historical registry is defined later in part_014c.inc. This extension is
// composed at the verified namespace boundary after Engine closes, so forward
// declarations preserve the public historical registry for the existing corpus:
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

// Reuse the canonical append/lookup abstraction and keep the concrete Garbodor
// scenario family beside that shared Forretress-side machinery.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
#include "core/forretress/scenario_extension.inc"
#include "core/forretress/garbodor_scenario_extension.inc"
