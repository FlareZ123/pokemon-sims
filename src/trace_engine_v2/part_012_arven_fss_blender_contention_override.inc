  bool arven_final_energy_vessel_dead_role_route_live() const {
    if (!deck_seen_ || !supporter_allowed() || hand_count(Card::Arven) == 0 ||
        item_locked() || scenario_.dci != DciProfile::NoDiscardControl ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_payload() ||
        grass_needed() + fire_needed() != 1 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0) {
      return false;
    }

    const bool needed_energy_searchable =
        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
    const bool dead_role_held = hand_count(Card::QuickBall) > 0 ||
        hand_count(Card::StevensResolve) > 0 ||
        hand_count(Card::LatiasEx) > 0 || hand_count(Card::Gladion) > 0;

    // Active Regidrago VSTAR, payload, and GF are already complete. Arven can
    // search Earthen Vessel, one now-dead setup role can legally pay its printed
    // discard cost, and the searched Basic Energy can use the unused manual
    // attachment for the earliest observable ready state:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1050
    return needed_energy_searchable && dead_role_held;
  }

  bool arven_fss_blender_contention_route_live() const {
    if (!deck_seen_ || !supporter_allowed() || hand_count(Card::Arven) == 0 ||
        item_locked() || state_.vstar_power_used || !can_play_payload_this_turn()) {
      return false;
    }

    const auto powered_evolvable_regi = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn &&
             pokemon.grass >= 2 && pokemon.fire >= 1 && pokemon.tool == Tool::None;
    };
    const bool active_route = state_.active && powered_evolvable_regi(*state_.active);
    const bool benched_route = can_free_retreat_with_latias() &&
        std::any_of(state_.bench.begin(), state_.bench.end(), powered_evolvable_regi);
    if (!active_route && !benched_route) return false;

    const auto in_deck = [this](const Card card) {
      return std::find(state_.deck.begin(), state_.deck.end(), card) != state_.deck.end();
    };
    const bool payload_in_deck = std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    return in_deck(Card::ForestSealStone) && in_deck(Card::RegidragoVstar) &&
           in_deck(Card::BrilliantBlender) && payload_in_deck;
  }

  bool play_arven() {
    if (arven_final_energy_vessel_dead_role_route_live()) {
      remove_one(state_.hand, Card::Arven);
      state_.discard.push_back(Card::Arven);
      state_.supporter_used = true;
      record_deck_search_knowledge("Arven");
      const bool found = move_deck_to_hand(Card::EarthenVessel);
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01",
            found ? "Searched Earthen Vessel for the final Basic Energy route."
                  : "Earthen Vessel was absent after legal deck inspection.");
      return true;
    }

    if (!arven_fss_blender_contention_route_live()) {
      return play_arven_fss_blender_contention_original();
    }

    // Arven's Item and Tool searches are separate resources. When a powered,
    // evolvable Regidrago V already has a legal Active route, Forest Seal Stone can
    // use Star Alchemy for Regidrago VSTAR while Brilliant Blender supplies the
    // remaining Dragon payload. Searching Mysterious Treasure here duplicates the
    // VSTAR connector and delays the earliest legal ready state:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Core evolution, Tool, Ability, Item, and retreat rules:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready and connector priorities:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/745
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");

    std::vector<Card> found;
    if (move_deck_to_hand(Card::BrilliantBlender)) found.push_back(Card::BrilliantBlender);
    if (move_deck_to_hand(Card::ForestSealStone)) found.push_back(Card::ForestSealStone);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched an Item and a Pokemon Tool without duplicating the VSTAR connector: " +
              join_cards(found) + ".");
    return true;
  }
