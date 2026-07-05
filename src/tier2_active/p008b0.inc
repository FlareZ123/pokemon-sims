  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0 || has_any_regi()) return false;
    // Let live normal Basic-search connectors act first.
    if (hand_count(Card::QuickBall) > 0 || hand_count(Card::MysteriousTreasure) > 0 || hand_count(Card::UltraBall) > 0 || hand_count(Card::Arven) > 0) return false;
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;
    const std::array<Card, 6> priorities{Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::MawileGX, Card::DialgaGX};
