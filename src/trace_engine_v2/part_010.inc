    const auto second = choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first), true);
    state_.hand.push_back(*first);
    if (!second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first, "Ultra Ball cost", "R-UB-01");
    discard_from_hand(*second, "Ultra Ball cost", "R-UB-01");
    // Comparator card-text source: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    record_deck_search_knowledge("Ultra Ball");

    const std::array<Card, 5> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (target == Card::TapuLeleGX && want_tapu);
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", "Searched a Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_ultra_ball_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Pokémon remained after search.");
    return true;
  }

  bool play_evolution_incense() {
    if (item_locked() || hand_count(Card::EvolutionIncense) == 0 || !need_vstar() || hand_count(Card::RegidragoVstar) > 0) return false;
    remove_one(state_.hand, Card::EvolutionIncense);
    state_.discard.push_back(Card::EvolutionIncense);
    // Comparator card-text source: https://api.pokemontcg.io/v2/cards/swsh1-163
    record_deck_search_knowledge("Evolution Incense");
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
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM", "Earthen Vessel");
    return true;
  }

  bool play_brilliant_blender() {
    // Blender can only discard cards found in deck: https://api.pokemontcg.io/v2/cards/sv8-164
    // After K1 proves every payload is absent from deck, preserve Blender for a
    // later shuffle instead of consuming the Item on a dead payload route.
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck()) return false;
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
      for (Pokemon& p : state_.bench) {
        if (is_pokemon_v(p.card) && p.tool == Tool::None) { target = &p; break; }
      }
    }
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::ForestSealStone);
    target->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL; R-GAME-ITEM", "Attached Forest Seal Stone to Regidrago V.");
    return true;
  }

  bool should_play_steven() const {
    if (state_.turn != 1 || scenario_.going_first || !has_regi_v()) return false;
    const int axes = static_cast<int>(need_vstar() && hand_count(Card::RegidragoVstar) == 0) +
                     static_cast<int>(need_energy() && hand_count(Card::Crispin) == 0) +
                     static_cast<int>(need_payload() && payload_might_be_in_deck() &&
                                      hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0);
    return axes >= 2;
  }

  bool can_fetch_oricorio_for_compression() const {
    return need_energy() && bench_space() > 0 && !in_play(Card::Oricorio) &&
           hand_count(Card::Oricorio) == 0 && deck_count_after_search_started(Card::Oricorio) > 0 &&
           deck_count_after_search_started(Card::Grass) + deck_count_after_search_started(Card::Fire) >= 2;
  }

  Card fss_target_after_search_started() const {
    if (state_.turn == 1 && !scenario_.going_first && should_play_steven() && deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;

    if (need_regi() && bench_space() > 0) {
      if (deck_count_after_search_started(Card::RegidragoV) > 0) return Card::RegidragoV;
      if (prize_count_after_reveal(Card::RegidragoV) > 0 && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;
    }
    if (need_vstar()) {
      if (deck_count_after_search_started(Card::RegidragoVstar) > 0) return Card::RegidragoVstar;
      if (prize_count_after_reveal(Card::RegidragoVstar) > 0 && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;
    }
    if (final_prized_energy_for_gladion().has_value() && deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;

    if (need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && deck_count_after_search_started(Card::LatiasEx) > 0) return Card::LatiasEx;
    if (need_active_vstar() && !need_energy() && !need_payload() &&
        deck_count_after_search_started(Card::TateLiza) > 0) return Card::TateLiza;
    if (need_payload() && payload_might_be_in_deck() && !item_locked() &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0) return Card::BrilliantBlender;
    if (need_payload() && payload_might_be_in_deck() && item_locked() &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0) return Card::ProfessorBurnet;
    if (need_energy() && can_fetch_oricorio_for_compression()) return Card::Oricorio;
    if (need_energy() && deck_count_after_search_started(Card::EarthenVessel) > 0 && is_legal_cost_available(can_play_payload_this_turn(), true)) return Card::EarthenVessel;
    if (need_energy() && crispin_can_advance_energy_axis() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;
    if (need_energy() && !state_.manual_energy_used) {
      // Star Alchemy may search the deck for any card: https://api.pokemontcg.io/v2/cards/swsh12-156
      // With no stronger Energy connector, take the missing Basic Energy that can be manually attached this turn.
      if (grass_needed() > 0 && deck_count_after_search_started(Card::Grass) > 0) return Card::Grass;
      if (fire_needed() > 0 && deck_count_after_search_started(Card::Fire) > 0) return Card::Fire;
    }
    if (deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    if (deck_count_after_search_started(Card::MysteriousTreasure) > 0) return Card::MysteriousTreasure;
    if (deck_count_after_search_started(Card::QuickBall) > 0) return Card::QuickBall;
    return Card::Serena;
  }
