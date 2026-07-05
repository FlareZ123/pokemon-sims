    return true;
  }

  bool play_evolution_incense() {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0 || !need_vstar() || hand_count(Card::RegidragoVstar) > 0) return false;
    remove_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    bool found = move_deck_to_hand(Card::RegidragoVstar);
    Card found_card = Card::RegidragoVstar;
    if (!found && move_deck_to_hand(Card::GoodraVstar)) { found = true; found_card = Card::GoodraVstar; }
    if (!found && move_deck_to_hand(Card::Dipplin)) { found = true; found_card = Card::Dipplin; }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EVOINCENSE-01; R-GAME-ITEM", found ? "Searched an Evolution Pokémon: " + std::string(name(found_card)) + "." : "No Evolution Pokémon remained in deck.");
    return true;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    if (item_locked() || hand_count(Card::EarthenVessel) == 0) return false;
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    if (!need_energy() && !can_jit_discard) return false;
    const auto cost = choose_discard(can_jit_discard, true);
    if (!cost) return false;
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(*cost, "Earthen Vessel cost", "R-EV-01");
