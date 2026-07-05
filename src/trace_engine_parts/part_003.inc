    Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active && state_.active->card == Card::RegidragoV && state_.active->entered_turn < state_.turn) {
      best = &*state_.active;
      best_score = score_regi(*best, true);
    }
    for (Pokemon& p : state_.bench) {
      if (p.card != Card::RegidragoV || p.entered_turn >= state_.turn) continue;
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    if (best == nullptr) return false;
    remove_one(state_.hand, Card::RegidragoVstar);
    best->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE", "Evolved " + std::string(name(Card::RegidragoV)) + " into Regidrago VSTAR.");
    return true;
  }

  bool retreat_to_benched_vstar_with_latias() {
    if (!need_active_vstar() || !can_free_retreat_with_latias()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    std::swap(*state_.active, *it);
    state_.retreat_used = true;
    trace("RETREAT", "R-LATIAS-01; R-GAME-RETREAT", "Latias ex gives the Basic Active no Retreat Cost; promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_switch() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || !need_active_vstar()) return false;
    auto it = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& p) { return p.card == Card::RegidragoVstar; });
    if (it == state_.bench.end()) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    std::swap(*state_.active, *it);
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza switch mode: promoted Benched Regidrago VSTAR.");
    return true;
  }

  bool play_tate_draw() {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 || state_.hand.size() >= 4U) return false;
    remove_one(state_.hand, Card::TateLiza);
    state_.discard.push_back(Card::TateLiza);
    state_.supporter_used = true;
    state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
    state_.hand.clear();
    shuffle(state_.deck);
    for (int draw = 0; draw < 5; ++draw) draw_card("Tate & Liza");
    trace("PLAY SUPPORTER", "R-TATE-01; R-GAME-SUPPORTER", "Tate & Liza draw mode: shuffled remaining hand and drew 5.");
    return true;
  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_any_regi()) return false;
    // Let live normal Basic-search connectors act first. This preserves Heavy Ball for a collapsed Basic prize state.
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;
    remove_one(state_.hand, Card::HisuianHeavyBall);
    const std::array<Card, 6> priorities{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::MawileGX, Card::DialgaGX};
    for (const Card desired : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), desired);
      if (it == state_.prizes.end()) continue;
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM", "Looked at Prizes and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM", "Looked at Prizes; did not reveal a useful Basic, so Heavy Ball was discarded.");
    return true;
  }

  bool need_regi() const { return !has_any_regi(); }
  bool need_vstar() const { return has_regi_v() && !has_vstar(); }

  bool has_other_payload_outlet(const Card excluded) const {
    // Only visible cards are used here. The current search card cannot be counted as its own later discard outlet.
    const auto present = [this, excluded](const Card card) { return card != excluded && hand_count(card) > 0; };
    return present(Card::MysteriousTreasure) || present(Card::QuickBall) || present(Card::UltraBall) ||
           present(Card::EarthenVessel) || present(Card::Serena) || present(Card::BrilliantBlender) ||
           (supporter_allowed() && present(Card::ProfessorBurnet));
  }


  Card fallback_mysterious_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  Card fallback_quick_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::MawileGX, Card::DialgaGX}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
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
    const bool want_latias = need_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
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
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::QuickBall) == 0) return false;
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                 std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool want_regi = need_regi();
    const bool want_latias = need_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector();
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 && needs_oricorio_connector();
    const bool want_payload = need_payload() && !can_jit_discard && has_other_payload_outlet(Card::QuickBall);
    if (!can_jit_discard && !want_regi && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    const auto cost = choose_discard(can_jit_discard, true);
    if (!cost) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(*cost, "Quick Ball cost", "R-QB-01");
    const std::array<Card, 6> targets{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::DialgaGX, Card::MawileGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (target == Card::DialgaGX && want_payload);
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", "Searched a Basic Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    const Card fallback = fallback_quick_ball_target_after_search_started();
    const bool found_fallback = move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Basic Pokémon remained after search.");
    return true;
  }

  bool play_ultra_ball(const bool permit_payload) {
    if (item_locked() || hand_count(Card::UltraBall) == 0) return false;
    const bool want_regi = need_regi();
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0;
    const bool want_latias = need_active_vstar() && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool want_oricorio = need_energy() && hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0;
    if (!want_regi && !want_vstar && !want_latias && !want_oricorio) return false;

    const auto first = choose_discard(permit_payload && can_play_payload_this_turn(), true);
    if (!first) return false;
    remove_one(state_.hand, *first);
    const auto second = choose_discard(permit_payload && can_play_payload_this_turn() && !is_payload(*first), true);
    state_.hand.push_back(*first);
    if (!second) return false;

    remove_one(state_.hand, Card::UltraBall);
    state_.discard.push_back(Card::UltraBall);
    discard_from_hand(*first, "Ultra Ball cost", "R-UB-01");
    discard_from_hand(*second, "Ultra Ball cost", "R-UB-01");

    const std::array<Card, 5> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::Oricorio && want_oricorio);
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
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 || !can_play_payload_this_turn()) return false;
    remove_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    std::vector<Card> discarded;
