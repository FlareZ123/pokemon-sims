#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

// Namespace-scope contract for the Forretress scenario registry. package.inc
// includes this header only after the shared Scenario model is available, keeping
// the feature contract separate from textual runtime composition.
// C++ declaration rules: https://eel.is/c++draft/dcl.pre
// Cleanup ownership plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;

ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);
