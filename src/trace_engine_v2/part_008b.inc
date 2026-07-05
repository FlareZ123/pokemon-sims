  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0) return false;

    const bool need_latias = need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0;
    const bool need_oricorio = need_energy() && !in_play(Card::Oricorio) && hand_count(Card::Oricorio) == 0 &&
                               bench_space() > 0 && needs_oricorio_connector();
    if (!need_regi() && !need_latias && !need_oricorio) return false;

    // A live ordinary Basic-search route is more flexible and should be used before Prize inspection.
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 ||
        hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;

    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;
    std::vector<Card> priorities;
    if (need_regi()) priorities.push_back(Card::RegidragoV);
    if (need_latias) priorities.push_back(Card::LatiasEx);
    if (need_oricorio) priorities.push_back(Card::Oricorio);
    for (const Card desired : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), desired);
      if (it == state_.prizes.end()) continue;
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
            "Looked at Prizes and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Looked at Prizes; no needed Basic was present, so Heavy Ball was discarded.");
    return true;
  }

  bool need_regi() const { return !has_any_regi(); }
  // The VSTAR card axis is complete once the card is in hand; evolve_best_regi
  // resolves the later game-state transition when its timing condition is legal.
  bool need_vstar() const { return has_regi_v() && !has_vstar() && hand_count(Card::RegidragoVstar) == 0; }

  bool has_other_payload_outlet(const Card excluded) const {
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
