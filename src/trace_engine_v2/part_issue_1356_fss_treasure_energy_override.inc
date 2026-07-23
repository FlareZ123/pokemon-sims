  int issue_1417_treasure_hold_turn_ = 0;

  bool issue_1356_treasure_vstar_energy_split_available() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !deck_seen_ || !state_.manual_energy_used ||
        !state_.vstar_power_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool fire_is_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0 && hand_count(Card::Grass) > 0;
    const bool grass_is_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::Fire) > 0;

    // Star Alchemy has supplied the missing next-turn Energy. The opposite held
    // Energy is now a legal dynamic-DCI cost for Mysterious Treasure, which
    // searches Regidrago VSTAR and preserves a second Treasure plus the held
    // Dragon for the T3 strict-JIT payload:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return fire_is_next_attachment || grass_is_next_attachment;
  }

  bool issue_1356_complete_next_turn_route_holds_quick_ball() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        !deck_seen_ || !state_.manual_energy_used || !state_.vstar_power_used ||
        item_locked() || !state_.active ||
        state_.active->entered_turn >= state_.turn ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool fire_is_held_next_attachment =
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0;
    const bool grass_is_held_next_attachment =
        state_.active->grass == 1 && state_.active->fire >= 1 &&
        hand_count(Card::Grass) > 0;
    const bool vstar_axis_complete = active_is_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool payable_treasure_bridge =
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        ((fire_is_held_next_attachment && hand_count(Card::Grass) > 0) ||
         (grass_is_held_next_attachment && hand_count(Card::Fire) > 0));

    // Quick Ball into Oricorio spends an Item, a second discard, a support
    // Pokémon, and a Bench slot after the Energy, VSTAR, and payload axes already
    // have complete public routes. It cannot improve the T3 deadline:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Bench, Ability, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Resource-preservation priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1335 https://github.com/FlareZ123/pokemon-sims/issues/1356
    return (vstar_axis_complete || payable_treasure_bridge) &&
        (fire_is_held_next_attachment || grass_is_held_next_attachment);
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1356_complete_next_turn_route_holds_quick_ball()) return false;
    return play_quick_ball_issue1356_original(permit_payload);
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_1417_treasure_hold_turn_ == state_.turn) {
      // The recovered Mysterious Treasure is a next-turn connector. Playing it on
      // the Legacy Star turn would discard the held Dragon before GGF is complete,
      // violating the repository strict-JIT timing contract:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Item and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
      return false;
    }
    if (!issue_1356_treasure_vstar_energy_split_available()) {
      return play_mysterious_treasure_issue1356_original(permit_payload);
    }

    const Card surplus_energy = state_.active->fire == 0 ? Card::Grass : Card::Fire;
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(surplus_energy, "Mysterious Treasure split-route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1356 surplus Energy cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("projected issue-1356 Regidrago VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded the surplus off-type Energy and searched Regidrago VSTAR; preserved the second Treasure for the T3 Dragon payload.");
    return true;
  }
