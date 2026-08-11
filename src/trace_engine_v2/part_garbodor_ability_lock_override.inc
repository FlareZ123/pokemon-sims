  bool garbodor_shake_scenario() const {
    return scenario_.label.rfind("garbodor-shake-ability-lock/", 0) == 0;
  }

  bool garbodor_abilities_locked() const {
    if (!garbodor_shake_scenario() || state_.path_lock_removed) return false;

    // Boost Shake evolves a Pokemon at the end of the opponent's turn. The modeled
    // opponent uses it on its first turn to establish Garbodor, then keeps a Tool on
    // Garbodor for Garbotoxin. Going first leaves our T1 before that opponent turn;
    // going second means the opponent's setup turn has already happened:
    // https://api.pokemontcg.io/v2/cards/swsh7-142
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return scenario_.going_first ? state_.turn >= 2 : state_.turn >= 1;
  }

  bool ability_available_for_pokemon_garbodor(const Card card) const {
    // Garbotoxin removes Abilities from every other Pokemon while Garbodor has a
    // Pokemon Tool attached. Path to the Peak remains a separate rule-box-only
    // suppression model:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_abilities_locked()) return false;
    return !is_rule_box_pokemon(card) || !rule_box_abilities_locked();
  }
