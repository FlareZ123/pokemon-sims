    return Card::RegidragoV;
  }

  Card fallback_ultra_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::MawileGX, Card::Dipplin}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi();
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool want_latias = need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector();
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 && needs_oricorio_connector();
    const bool want_payload = need_payload() && !can_jit_discard && has_other_payload_outlet(Card::MysteriousTreasure);
    if (!can_jit_discard && !want_regi && !want_vstar && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    const auto cost = choose_discard(can_jit_discard, true);
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(*cost, "Mysterious Treasure cost", "R-MT-01");

    // The deck is now being legally searched. Select a legal target after inspecting it.
    const std::array<Card, 9> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::TapuLeleGX,
                                      Card::Oricorio, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (want_payload && is_payload(target));
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", "Searched a Psychic or Dragon Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_mysterious_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Psychic or Dragon Pokémon remained after search.");
