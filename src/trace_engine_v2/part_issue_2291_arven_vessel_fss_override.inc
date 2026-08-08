  bool play_arven() {
    if (!issue_2291_wonder_tag_arven_fss_vessel_route() || hand_count(Card::Arven) == 0) {
      return play_arven_issue2291_original();
    }

    // Arven searches one Item and one Pokemon Tool. In this exact K1 state,
    // Earthen Vessel is the Item channel that supplies the missing Grass while
    // Forest Seal Stone supplies Star Alchemy for the missing Regidrago VSTAR.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core search, Supporter, Item, Tool, Ability, Energy attachment and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route and connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-2291 route");

    std::vector<Card> found;
    if (move_deck_to_hand(Card::EarthenVessel)) found.push_back(Card::EarthenVessel);
    if (move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01; P-DCI-01",
          "Searched the deterministic K1 FSS/Vessel finish: " + join_cards(found) + ".");
    return true;
  }
