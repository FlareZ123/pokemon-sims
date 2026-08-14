#pragma once

// Canonical namespace-scope composition boundary for the Forretress feature.
// Keep the runtime and scenario extension adjacent here while Engine is closed.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "core/forretress/runtime.inc"

// The historical registry is defined later in part_014c.inc. Keep the scenario
// extension at this verified namespace boundary after Engine closes, so forward
// declarations preserve the public historical registry for the existing corpus:
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

const GarbodorScenarioSet kGarbodorScenarios{{
    {"garbodor-shake-ability-lock/go-first",
     DciProfile::StrictJit, LockMode::None, true, 5},
    {"garbodor-shake-ability-lock/go-second",
     DciProfile::StrictJit, LockMode::None, false, 5},
}};

template <typename ScenarioRange>
void append_scenarios(ScenarioList& result, const ScenarioRange& scenarios) {
  result.reserve(result.size() + scenarios.size());
  result.insert(result.end(), scenarios.begin(), scenarios.end());
}

const Scenario* find_garbodor_scenario(const std::string& label) {
  const auto match = std::find_if(
      kGarbodorScenarios.begin(), kGarbodorScenarios.end(),
      [&label](const Scenario& scenario) { return scenario.label == label; });
  return match == kGarbodorScenarios.end() ? nullptr : &*match;
}

ScenarioList all_scenarios_with_garbodor() {
  ScenarioList result = all_scenarios();
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // establishes Garbodor during the opponent's first turn, producing different
  // Regidrago T1 timing by seat. Append these rows so existing scenario indexes and
  // seeded common-random-number comparisons remain stable:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
  append_scenarios(result, kGarbodorScenarios);
  return result;
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  if (const Scenario* match = find_garbodor_scenario(label)) return *match;
  return scenario_by_label(label);
}
