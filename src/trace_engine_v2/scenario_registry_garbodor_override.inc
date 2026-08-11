// The historical registry is defined later in part_014c.inc. This wrapper is
// composed at a verified namespace boundary after Engine closes, so a forward
// declaration is sufficient while preserving the public historical registry for
// the existing test corpus:
// https://github.com/FlareZ123/pokemon-sims/issues/2808
std::vector<Scenario> all_scenarios();

std::vector<Scenario> all_scenarios_with_garbodor() {
  std::vector<Scenario> result = all_scenarios();
  // Garbodor BKP 57 suppresses Pokemon Abilities while it has a Tool. Boost Shake
  // establishes Garbodor during the opponent's first turn, producing different
  // Regidrago T1 timing by seat. Append these rows so existing scenario indexes and
  // seeded common-random-number comparisons remain stable:
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
