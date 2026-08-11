  std::string current_deck_id() const {
    if (total_copies(Card::DoubleDragonEnergy) > 0) {
      return "regidrago-dde-model";  // Modeling recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238 ; DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    }
    if (total_copies(Card::CrobatV) > 0) {
      return "modeling-crobat-variant";
    }
    if (total_copies(Card::SecretBox) > 0 && total_copies(Card::Pineco) > 0) {
      return "regidrago-pineco";
    }
    if (total_copies(Card::RegidragoV) == 4 &&
        total_copies(Card::BrilliantBlender) == 1) {
      return "regidrago-shell";
    }
    return "custom";
  }

  std::string dci_name() const {
    switch (scenario_.dci) {
      case DciProfile::StrictJit: return "strict-JIT";
      case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
      case DciProfile::NoDiscardControl: return "no-discard-control";
    }
    return "unknown";
  }

  std::string lock_name() const {
    switch (scenario_.locks) {
      case LockMode::None: return "none";
      case LockMode::TurnTwoItem: return "turn-two Item lock";
      case LockMode::FullItem: return "full Item lock";
      case LockMode::FullRuleBoxAbility: return "full Rule Box Ability lock";
      case LockMode::FullCombined: return "full Item + Rule Box Ability lock";
      case LockMode::FullSupporter: return "full Supporter lock";
    }
    return "unknown";
  }
