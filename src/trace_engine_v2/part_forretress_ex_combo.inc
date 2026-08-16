#pragma once

// Canonical namespace-scope composition boundary for the Forretress feature.
// Keep runtime and scenario extension ownership adjacent here while Engine is closed.
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

// Reuse the canonical append/lookup abstraction instead of maintaining a second
// root-level copy beside feature-specific scenario data.
// Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/forretress/scenario_extension.inc
#include "core/forretress/scenario_extension.inc"

// Keep the Garbodor rows inside their sole append/lookup owner instead of exposing
// a second namespace-scope scenario container with coupled lifetime and ordering.
// Scenario specification: https://github.com/FlareZ123/pokemon-sims/issues/2808
const ScenarioExtension<GarbodorScenarioSet> kGarbodorScenarioExtension{
    GarbodorScenarioSet{{
        {"garbodor-shake-ability-lock/go-first",
         DciProfile::StrictJit, LockMode::None, true, 5},
        {"garbodor-shake-ability-lock/go-second",
         DciProfile::StrictJit, LockMode::None, false, 5},
    }}};

ScenarioList all_scenarios_with_garbodor() {
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // establishes Garbodor during the opponent's first turn, producing different
  // Regidrago T1 timing by seat. Append these rows so existing scenario indexes and
  // seeded common-random-number comparisons remain stable:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
  return kGarbodorScenarioExtension.append_to(all_scenarios());
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  if (const auto match = kGarbodorScenarioExtension.find(label)) {
    return match;
  }
  return scenario_by_label(label);
}
