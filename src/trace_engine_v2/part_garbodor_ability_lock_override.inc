  bool garbodor_shake_scenario() const {
    return scenario_.label.rfind("garbodor-shake-ability-lock/", 0) == 0;
  }

  bool garbodor_abilities_locked() const {
    if (!garbodor_shake_scenario() || state_.path_lock_removed) return false;

    // Boost Shake can evolve on the opponent's first turn and then ends that turn.
    // When Regidrago goes first, its T1 precedes that opponent turn, so Garbotoxin
    // starts on Regidrago T2. When Regidrago goes second, the opponent has already
    // completed the Boost Shake turn, so Garbotoxin starts on Regidrago T1:
    // https://api.pokemontcg.io/v2/cards/swsh7-142
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return scenario_.going_first ? state_.turn >= 2 : state_.turn >= 1;
  }

  bool ability_available_for_pokemon_garbodor(const Card card) const {
    // Garbotoxin removes Abilities from every other Pokemon while Garbodor has a
    // Tool attached. Preserve the existing Path rule separately because Path only
    // suppresses Rule Box Pokemon Abilities:
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (garbodor_abilities_locked()) return false;
    return !is_rule_box_pokemon(card) || !rule_box_abilities_locked();
  }
