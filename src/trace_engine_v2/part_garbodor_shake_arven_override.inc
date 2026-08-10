  bool arven_garbodor_field_blower_route_live() {
    if (!supporter_allowed() || item_locked() || hand_count(Card::Arven) == 0 ||
        !garbodor_abilities_locked() || !might_be_unseen(Card::FieldBlower)) {
      return false;
    }

    // Spending Arven consumes this turn's Supporter action, so this projection
    // intentionally excludes Wonder Tag-only lines. It admits only an Ability route
    // that remains useful after Arven resolves, such as Vital Dance, Skyliner,
    // Dark Asset, or Legacy Star.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Core Supporter, Item, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    return garbodor_unlock_has_live_ability_route(true);
  }

  bool play_arven() {
    if (!arven_garbodor_field_blower_route_live()) {
      return play_arven_pre_garbodor_shake();
    }

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven Garbodor lock-answer route");

    const bool found_field_blower = move_deck_to_hand(Card::FieldBlower);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-GARBODOR-SHAKE-01",
          found_field_blower
              ? "Searched Field Blower as the Garbodor lock answer and voluntarily left the Tool search unresolved."
              : "Field Blower was unavailable after Arven legally inspected the deck.");
    if (!found_field_blower) return true;

    // Arven fully resolves before the searched Item is played. Field Blower then
    // removes Garbodor's attached Tool and restores Pokemon Abilities for the rest
    // of this turn only. The opponent is assumed to re-Tool before our next turn.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
    // Core Supporter resolution and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2808
    if (!remove_one(state_.hand, Card::FieldBlower)) {
      throw std::logic_error("Arven Garbodor Field Blower target disappeared");
    }
    state_.discard.push_back(Card::FieldBlower);
    garbodor_unlocked_turn_ = state_.turn;
    trace("PLAY ITEM", "R-GAME-ITEM; P-GARBODOR-SHAKE-01",
          "Field Blower discarded Garbodor's attached Tool after Arven resolved; Garbotoxin is off for this turn and returns before the next player turn under the maximum-pressure re-Tool assumption.");
    return true;
  }
