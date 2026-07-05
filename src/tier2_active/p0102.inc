    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM", "Earthen Vessel");
    return true;
  }

  bool play_brilliant_blender() {
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !can_play_payload_this_turn()) return false;
    remove_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    std::vector<Card> discarded;
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(payload)) { discarded.push_back(payload); break; }
    }
    if (move_deck_to_discard(Card::Dipplin)) discarded.push_back(Card::Dipplin);
    shuffle(state_.deck);
    outcome_.used_blender = true;
    trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-DCI-01", "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  bool attach_fss() {
    if (item_locked() || hand_count(Card::ForestSealStone) == 0) return false;
    Pokemon* target = nullptr;
    if (state_.active && is_pokemon_v(state_.active->card) && state_.active->tool == Tool::None) target = &*state_.active;
    if (target == nullptr) {
