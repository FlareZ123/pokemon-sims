  bool arven_powerglass_tool_route_live() const {
    if (!state_.active || state_.active->tool != Tool::None ||
        (state_.active->card != Card::RegidragoV && state_.active->card != Card::RegidragoVstar)) {
      return false;
    }

    // Forest Seal Stone remains the higher-priority Tool while its one-per-game VSTAR
    // Power can still be used. Once that route is spent or known unavailable, Arven may
    // search Powerglass for an Active-holder Energy recovery line:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const bool forest_seal_route_available = !state_.vstar_power_used &&
        (hand_count(Card::ForestSealStone) > 0 || might_be_unseen(Card::ForestSealStone));
    if (forest_seal_route_available || !might_be_unseen(Card::Powerglass)) return false;

    const bool needed_grass_in_discard =
        state_.active->grass < 2 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard =
        state_.active->fire < 1 && count_of(state_.discard, Card::Fire) > 0;
    return needed_grass_in_discard || needed_fire_in_discard;
  }

  bool play_arven() {
    if (!supporter_allowed() || hand_count(Card::Arven) == 0 ||
        !arven_powerglass_tool_route_live()) {
      return play_arven_powerglass_original();
    }

    // Arven may search for a Pokémon Tool, and Powerglass remains a Pokémon Tool
    // rather than an Item under the Tool erratum, so this route is legal through
    // Item lock. Powerglass then checks its Active holder at end of turn:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");
    const bool found = move_deck_to_hand(Card::Powerglass);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; R-POWERGLASS-01",
          found ? "Searched Powerglass as the live Pokémon Tool target."
                : "Powerglass was not in the deck after legal search.");
    return true;
  }
