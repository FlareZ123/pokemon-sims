// The historical registry is defined later in part_014c.inc. This extension is
// composed at the verified namespace boundary after Engine closes, so forward
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
  result.reserve(result.size() + kGarbodorScenarios.size());
  result.insert(result.end(), kGarbodorScenarios.begin(), kGarbodorScenarios.end());
  return result;
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  if (const Scenario* match = find_garbodor_scenario(label)) return *match;
  return scenario_by_label(label);
}
