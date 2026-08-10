std::vector<Scenario> all_scenarios_with_garbodor() {
  std::vector<Scenario> result = all_scenarios();
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // lets the opponent establish that Evolution on its first turn, which produces
  // different Regidrago T1 timing by seat:
  // https://api.pokemontcg.io/v2/cards/xy9-57
  // https://api.pokemontcg.io/v2/cards/swsh7-142
  // https://github.com/FlareZ123/pokemon-sims/issues/2808
  result.push_back({"garbodor-shake-ability-lock/go-first",
                    DciProfile::StrictJit, LockMode::None, true, 5});
  result.push_back({"garbodor-shake-ability-lock/go-second",
                    DciProfile::StrictJit, LockMode::None, false, 5});
  return result;
}

std::optional<Scenario> scenario_by_label_with_garbodor(const std::string& label) {
  for (const Scenario& scenario : all_scenarios_with_garbodor()) {
    if (scenario.label == label) return scenario;
  }
  return std::nullopt;
}
