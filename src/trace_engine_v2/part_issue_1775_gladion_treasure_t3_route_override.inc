  bool issue_1775_gladion_treasure_t3_route_available() const {
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    return deck_seen_ && prizes_known() && strict_payload_timing() &&
        supporter_allowed() && !item_locked() && state_.turn == 3 &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        !state_.manual_energy_used && need_energy() && need_vstar() &&
        need_payload() && hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::Gladion) > 0 && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::MysteriousTreasure) == 0 &&
        hand_count(Card::RegidragoVstar) == 0 && held_payload &&
        prize_count_after_reveal(Card::MysteriousTreasure) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0;
  }

  bool play_issue_1775_gladion_treasure_t3_route() {
    if (!issue_1775_gladion_treasure_t3_route_available()) return false;

    // Earthen Vessel spends the route-replaced Steven while its printed search
    // supplies the final Grass. Gladion then recovers the known prized Treasure,
    // whose one-card cost is the held Dragon payload and whose search supplies the
    // VSTAR. The prior-turn Active Basic may evolve immediately, so this complete
    // K1 line reaches the strict-JIT ready state on T3. Brilliant Blender solves
    // only the payload axis and delays the missing VSTAR axis:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, search, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, supporter contention, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1775
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(Card::StevensResolve,
                           "Earthen Vessel issue-1775 route cost",
                           "R-EV-01; R-STEVEN-01; P-DCI-01")) {
      throw std::logic_error("Issue-1775 Steven cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1775 route");
    std::vector<Card> found_energy;
    for (int copy = 0; copy < 2; ++copy) {
      if (!move_deck_to_hand(Card::Grass)) break;
      found_energy.push_back(Card::Grass);
    }
    shuffle(state_.deck);
    trace("Earthen Vessel", "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Discarded Steven's Resolve and searched: " + join_cards(found_energy) + ".");

    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1775 searched Grass disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-RVS-01",
          "Grass Energy manually to the prior-turn Active Regidrago V.");

    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto prized_treasure = std::find(
        state_.prizes.begin(), state_.prizes.end(), Card::MysteriousTreasure);
    if (prized_treasure == state_.prizes.end()) {
      throw std::logic_error("Issue-1775 prized Treasure disappeared");
    }
    *prized_treasure = Card::Gladion;
    state_.hand.push_back(Card::MysteriousTreasure);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Exchanged Gladion for the known prized Mysterious Treasure.");

    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    if (payload == state_.hand.end()) {
      throw std::logic_error("Issue-1775 held payload disappeared");
    }
    const Card selected_payload = *payload;
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(selected_payload,
                           "Mysterious Treasure issue-1775 route cost",
                           "R-MT-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1775 payload cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1775 route");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("Issue-1775 VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01",
          "Discarded " + std::string(name(selected_payload)) +
              " and searched Regidrago VSTAR.");

    if (!evolve_best_regi()) {
      throw std::logic_error("Issue-1775 evolution disappeared");
    }
    return true;
  }
