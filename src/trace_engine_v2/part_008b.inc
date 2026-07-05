  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_any_regi()) return false;
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;
    const std::array<Card, 6> priorities{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::MawileGX, Card::DialgaGX};
    for (const Card desired : priorities) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), desired);
      if (it == state_.prizes.end()) continue;
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01", "Looked at Prizes and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01", "Looked at Prizes; did not reveal a useful Basic, so Heavy Ball was discarded.");
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
