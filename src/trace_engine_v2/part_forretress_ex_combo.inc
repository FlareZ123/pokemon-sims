// Canonical namespace-scope composition boundary for the Forretress feature.
// Keep the runtime and scenario extension adjacent here while Engine is closed.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
#include "core/forretress/runtime.inc"

// The historical registry is defined later in part_014c.inc. This extension is
// composed at the verified namespace boundary after Engine closes, so forward
// declarations preserve the public historical registry for the existing corpus:
// Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
std::vector<Scenario> all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);

const std::array<Scenario, 2> kGarbodorScenarios{{
    {"garbodor-shake-ability-lock/go-first",
     DciProfile::StrictJit, LockMode::None, true, 5},
    {"garbodor-shake-ability-lock/go-second",
     DciProfile::StrictJit, LockMode::None, false, 5},
}};

std::vector<Scenario> all_scenarios_with_garbodor() {
  std::vector<Scenario> result = all_scenarios();
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // establishes Garbodor during the opponent's first turn, producing different
  // Regidrago T1 timing by seat. Append these rows so existing scenario indexes and
  // seeded common-random-number comparisons remain stable:
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
  result.insert(result.end(), kGarbodorScenarios.begin(), kGarbodorScenarios.end());
  return result;
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  const auto match = std::find_if(
      kGarbodorScenarios.begin(), kGarbodorScenarios.end(),
      [&label](const Scenario& scenario) { return scenario.label == label; });
  if (match != kGarbodorScenarios.end()) return *match;
  return scenario_by_label(label);
}
