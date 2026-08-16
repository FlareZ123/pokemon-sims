#pragma once

// Public namespace-scope contract required by the Forretress scenario extensions.
// Keep these declarations separate from runtime mechanics so package composition
// does not mix simulator registry declarations with card-effect implementation.
// Cleanup plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
using ScenarioList = std::vector<Scenario>;
using GarbodorScenarioSet = std::array<Scenario, 2>;
ScenarioList all_scenarios();
std::optional<Scenario> scenario_by_label(const std::string& label);
