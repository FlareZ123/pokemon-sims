#pragma once

// Canonical namespace-scope composition package for the Forretress feature.
// Runtime and scenario-extension ownership stay together under core/forretress
// while Engine is closed, preserving the historical declaration and include order.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "runtime.inc"

// Public namespace-scope contract required by the Forretress scenario extension.
// Keep these declarations at the package boundary before the shared registry policy
// so this owner exposes its complete namespace-scope composition in one file.
// C++ declaration rules: https://eel.is/c++draft/dcl.pre
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

// Reuse the shared physical append/lookup and extension-first fallback mechanics.
// Shared scenario policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/scenario_extension_policy.inc
#include "../scenario_extension_policy.inc"

// Keep the concrete Garbodor / Boost Shake scenario family at the Forretress
// package boundary instead of forwarding through another feature-local .inc file.
// Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
// Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
// Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
const ScenarioExtension<GarbodorScenarioSet> kGarbodorScenarioExtension{
    GarbodorScenarioSet{{
        {"garbodor-shake-ability-lock/go-first",
         DciProfile::StrictJit, LockMode::None, true, 5},
        {"garbodor-shake-ability-lock/go-second",
         DciProfile::StrictJit, LockMode::None, false, 5},
    }}};

ScenarioList all_scenarios_with_garbodor() {
  // Append these rows so existing scenario indexes and seeded common-random-number
  // comparisons remain stable. Garbotoxin suppresses Pokemon Abilities while
  // Garbodor has a Pokemon Tool attached, and Boost Shake establishes the evolved
  // Pokemon during the opponent's first turn:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Advanced Ability and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  return kGarbodorScenarioExtension.append_to(all_scenarios());
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  return kGarbodorScenarioExtension.find_or_fallback(
      label, [&label] { return scenario_by_label(label); });
}
