
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
