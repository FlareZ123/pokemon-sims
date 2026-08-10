std::vector<Scenario> all_scenarios() {
  std::vector<Scenario> result = all_scenarios_pre_garbodor_shake();

  // Append these rows so every historical scenario keeps its existing index and
  // derived RNG stream. Garbotoxin is modeled separately from LockMode because it
  // suppresses Pokemon Abilities without prohibiting Items, Supporters, Stadiums,
  // Tools, or attachments.
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
  // Core turn and Trainer procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
  result.push_back({"strict-jit-garbodor-shake-ability-lock/go-first",
                    DciProfile::StrictJit, LockMode::None, true, 5});
  result.push_back({"strict-jit-garbodor-shake-ability-lock/go-second",
                    DciProfile::StrictJit, LockMode::None, false, 5});
  return result;
}

std::optional<Scenario> scenario_by_label(const std::string& label) {
  const std::string normalized =
      label == "garbodor-shake-ability-lock/go-first"
          ? "strict-jit-garbodor-shake-ability-lock/go-first"
          : label == "garbodor-shake-ability-lock/go-second"
              ? "strict-jit-garbodor-shake-ability-lock/go-second"
              : label;
  for (const Scenario& scenario : all_scenarios()) {
    if (scenario.label == normalized) return scenario;
  }
  return std::nullopt;
}
