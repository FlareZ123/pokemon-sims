  bool has_payable_team_yell_vstar_search() {
    if (item_locked()) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::TeamYellsCheer)) return false;

    // Evolution Incense searches an Evolution Pokémon without a discard cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    if (hand_count(Card::EvolutionIncense) > 0) {
      restore_hand();
      return true;
    }

    // Mysterious Treasure must discard another hand card before searching a Dragon
    // Pokémon. Test the cost after removing the Supporter that will already be played:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    const bool permit_payload = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload, true, true, Card::MysteriousTreasure).has_value()) {
      restore_hand();
      return true;
    }

    // Ultra Ball must still have two other policy-legal cards after Team Yell's Cheer
    // leaves the hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) > 0) {
      remove_one(state_.hand, Card::UltraBall);
      const auto first = choose_discard(permit_payload, true);
      if (first && remove_one(state_.hand, *first)) {
        const auto second = choose_discard(permit_payload && !is_payload(*first), true);
        if (second) {
          restore_hand();
          return true;
        }
      }
    }

    restore_hand();
    return false;
  }

  bool play_team_yell_vstar_recovery() {
    if (!supporter_allowed() || hand_count(Card::TeamYellsCheer) == 0 || !need_vstar() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        count_of(state_.discard, Card::RegidragoVstar) == 0) {
      return false;
    }

    // Preserve a direct deck search when a Regidrago VSTAR may already remain in the
    // deck. This recovery is selected after legal knowledge proves the search target
    // absent and a discarded copy can restore a payable same-turn route.
    if (might_be_unseen(Card::RegidragoVstar) || !has_payable_team_yell_vstar_search()) return false;

    remove_one(state_.hand, Card::TeamYellsCheer);
    state_.discard.push_back(Card::TeamYellsCheer);
    state_.supporter_used = true;

    // Team Yell's Cheer may shuffle up to 3 Pokémon and/or Supporter cards other than
    // itself from discard into the deck. Recover only the VSTAR required by this route:
    // https://api.pokemontcg.io/v2/cards/swsh9-149
    if (!remove_one(state_.discard, Card::RegidragoVstar)) {
      throw std::logic_error("Team Yell's Cheer VSTAR target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(),
                                   Card::RegidragoVstar);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(Card::RegidragoVstar);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Team Yell's Cheer shuffled Regidrago VSTAR from discard into the deck for a payable same-turn search.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (play_team_yell_vstar_recovery()) return;
    choose_supporter_original();
  }
