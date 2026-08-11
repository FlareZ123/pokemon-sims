  static constexpr std::string_view kGarbodorShakeScenarioPrefix =
      "garbodor-shake-ability-lock/";

  bool garbodor_shake_scenario() const {
    return scenario_.label.starts_with(kGarbodorShakeScenarioPrefix);
  }

  int garbodor_lock_activation_turn() const {
    return scenario_.going_first ? 2 : 1;
  }

  bool garbodor_lock_removed() const {
    // The registered Garbodor model reuses the shared lock-removal state:
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return state_.path_lock_removed;
  }

  bool garbodor_lock_established() const {
    // Boost Shake evolves a Pokemon at the end of the opponent's turn. The modeled
    // opponent uses it on its first turn to establish Garbodor, then keeps a Tool on
    // Garbodor for Garbotoxin. Going first leaves our T1 before that opponent turn;
    // going second means the opponent's setup turn has already happened:
    // https://api.pokemontcg.io/v2/cards/swsh7-142
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return state_.turn >= garbodor_lock_activation_turn();
  }

  bool garbodor_lock_present() const {
    return garbodor_shake_scenario() && garbodor_lock_established();
  }

  bool garbodor_abilities_locked() const {
    return garbodor_lock_present() && !garbodor_lock_removed();
  }

  bool rule_box_ability_available(const Card card) const {
    // Path to the Peak suppresses Rule Box Pokemon Abilities in the separate
    // registered Rule Box lock model:
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return !is_rule_box_pokemon(card) || !rule_box_abilities_locked();
  }

  bool ability_available_for_pokemon_garbodor(const Card card) const {
    // Garbotoxin removes Abilities from every other Pokemon while Garbodor has a
    // Pokemon Tool attached. Path to the Peak remains a separate rule-box-only
    // suppression model:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_abilities_locked()) return false;
    return rule_box_ability_available(card);
  }