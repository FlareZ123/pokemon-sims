  struct SimulationDeckIdentity {
    [[nodiscard]] static bool contains(const Engine& engine, const Card card) {
      return engine.total_copies(card) > 0;
    }

    [[nodiscard]] static bool is_regidrago_shell(const Engine& engine) {
      return engine.total_copies(Card::RegidragoV) == 4 &&
             engine.total_copies(Card::BrilliantBlender) == 1;
    }

    [[nodiscard]] static std::string id(const Engine& engine) {
      if (contains(engine, Card::DoubleDragonEnergy)) {
        // Modeling recipe: https://github.com/FlareZ123/pokemon-sims/issues/2238
        // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        return "regidrago-dde-model";
      }
      if (contains(engine, Card::CrobatV)) {
        return "modeling-crobat-variant";
      }
      if (contains(engine, Card::SecretBox) && contains(engine, Card::Pineco)) {
        return "regidrago-pineco";
      }
      if (is_regidrago_shell(engine)) {
        return "regidrago-shell";
      }
      return "custom";
    }
  };

  struct ScenarioLabelPolicy {
    [[nodiscard]] static const char* dci(const DciProfile profile) {
      // DCI/JIT policy vocabulary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      switch (profile) {
        case DciProfile::StrictJit: return "strict-JIT";
        case DciProfile::MatchupFlexJit: return "matchup-flex-JIT";
        case DciProfile::NoDiscardControl: return "no-discard-control";
      }
      return "unknown";
    }

    [[nodiscard]] static const char* lock(const LockMode mode) {
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
  };

  bool simulation_deck_contains(Card card) const {
    return SimulationDeckIdentity::contains(*this, card);
  }

  bool is_regidrago_shell_recipe() const {
    return SimulationDeckIdentity::is_regidrago_shell(*this);
  }

  std::string current_deck_id() const {
    return SimulationDeckIdentity::id(*this);
  }

  static const char* dci_label(DciProfile profile) {
    return ScenarioLabelPolicy::dci(profile);
  }

  static const char* lock_label(LockMode mode) {
    return ScenarioLabelPolicy::lock(mode);
  }

  std::string dci_name() const {
    return dci_label(scenario_.dci);
  }

  std::string lock_name() const {
    return lock_label(scenario_.locks);
  }
