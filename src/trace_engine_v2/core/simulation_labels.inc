  bool simulation_deck_contains(Card card) const {
    return total_copies(card) > 0;
  }

  bool is_regidrago_shell_recipe() const {
    return total_copies(Card::RegidragoV) == 4 &&
           total_copies(Card::BrilliantBlender) == 1;
  }

  std::string current_deck_id() const {
    if (simulation_deck_contains(Card::DoubleDragonEnergy)) {
      return "regidrago-dde-model";  // Modeling recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238 ; DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    }
    if (simulation_deck_contains(Card::CrobatV)) {
      return "modeling-crobat-variant";
    }
    if (simulation_deck_contains(Card::SecretBox) &&
        simulation_deck_contains(Card::Pineco)) {
      return "regidrago-pineco";
    }
    if (is_regidrago_shell_recipe()) {
      return "regidrago-shell";
    }
    return "custom";
  }

  static const char* dci_label(DciProfile profile) {
    // DCI/JIT policy vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    switch (profile) {
      case DciProfile::StrictJit: return "strict-JIT";
      case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
      case DciProfile::NoDiscardControl: return "no-discard-control";
    }
    return "unknown";
  }

  static const char* lock_label(LockMode mode) {
    // Scenario lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    switch (mode) {
      case LockMode::None: return "none";
      case LockMode::TurnTwoItem: return "turn-two Item lock";
      case LockMode::FullItem: return "full Item lock";
      case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
      case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
      case LockMode::FullSupporter: return "full Supporter lock";
    }
    return "unknown";
  }

  std::string dci_name() const {
    return dci_label(scenario_.dci);
  }

  std::string lock_name() const {
    return lock_label(scenario_.locks);
  }
