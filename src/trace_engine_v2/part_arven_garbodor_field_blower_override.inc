  bool arven_garbodor_field_blower_route_live() {
    if (!garbodor_abilities_locked() || item_locked() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || hand_count(Card::FieldBlower) > 0 ||
        !deck_search_available() || !might_be_unseen(Card::FieldBlower)) {
      return false;
    }

    // Arven may search one Item. Search Field Blower only when removing Garbodor's
    // attached Tool unlocks a live Pokemon-Ability setup route, preserving Field
    // Blower's matchup-dependent discrete value and Supporter contention:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/xy9-57
    // https://github.com/FlareZ123/pokemon-sims/issues/2808
    return garbodor_removal_unlocks_live_pokemon_route();
  }

  bool play_arven() {
    if (!arven_garbodor_field_blower_route_live()) {
      return play_arven_garbodor_original();
    }

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven Garbodor Field Blower route");

    std::vector<Card> found;
    if (move_deck_to_hand(Card::FieldBlower)) found.push_back(Card::FieldBlower);

    // Arven may also search one Pokemon Tool. Preserve the established Forest Seal
    // Stone route when Star Alchemy is unused and a Pokemon V has an open Tool slot.
    // Star Alchemy is the Tool's Ability under the repository's existing ruling:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    const bool open_v_tool_slot =
        (state_.active && is_pokemon_v(state_.active->card) &&
         state_.active->tool == Tool::None) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
        });
    if (!state_.vstar_power_used && open_v_tool_slot &&
        move_deck_to_hand(Card::ForestSealStone)) {
      found.push_back(Card::ForestSealStone);
    }

    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-CONNECTOR-01",
          "Arven searched the live Garbotoxin answer: " + join_cards(found) + ".");
    return true;
  }
