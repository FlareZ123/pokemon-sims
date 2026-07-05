  }

  bool play_heavy_ball() {
    if (item_locked() || hand_count(Card::HisuianHeavyBall) == 0) return false;

    const bool known_no_basic_prize = prizes_known() &&
        std::none_of(state_.prizes.begin(), state_.prizes.end(), is_basic);
    if (known_no_basic_prize) {
      trace("HOLD ITEM", "R-HEAVYBALL-01; P-KNOWLEDGE-01",
            "Known Prize cards contain no Basic Pokémon; retained Heavy Ball as a legal discard cost.");
      return false;
    }

    // At K0, Heavy Ball is an information action as well as a possible recovery.
    // It is played before ordinary search Items. After inspection, any revealed
    // Basic can be selected for its immediate connector value or as future fodder.
    remove_one(state_.hand, Card::HisuianHeavyBall);
    prizes_revealed_ = true;

    const auto prized = [this](const Card card) {
      return std::find(state_.prizes.begin(), state_.prizes.end(), card) != state_.prizes.end();
    };
    Card selected = Card::HisuianHeavyBall;

    // Direct current-turn axes first.
    if (need_regi() && prized(Card::RegidragoV)) selected = Card::RegidragoV;
    else if (needs_tapu_connector() && prized(Card::TapuLeleGX)) selected = Card::TapuLeleGX;
    else if (needs_oricorio_connector() && prized(Card::Oricorio)) selected = Card::Oricorio;
    else if (need_active_vstar() && ability_available_for_pokemon(Card::LatiasEx) && prized(Card::LatiasEx)) selected = Card::LatiasEx;

    // Prefer future connectors whose abilities can function in this scenario.
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::Oricorio, Card::TapuLeleGX,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if ((candidate == Card::TapuLeleGX || candidate == Card::LatiasEx) &&
            !ability_available_for_pokemon(candidate)) continue;
        if (prized(candidate)) { selected = candidate; break; }
      }
    }

    // If all functional connector choices are absent, still take any Basic as
    // future legal discard fodder rather than leave it prized.
    if (selected == Card::HisuianHeavyBall) {
      for (const Card candidate : {Card::RegidragoV, Card::TapuLeleGX, Card::Oricorio,
                                   Card::LatiasEx, Card::DialgaGX, Card::MawileGX}) {
        if (prized(candidate)) { selected = candidate; break; }
      }
    }

    if (selected != Card::HisuianHeavyBall) {
      const auto it = std::find(state_.prizes.begin(), state_.prizes.end(), selected);
      const Card revealed = *it;
      *it = Card::HisuianHeavyBall;
      state_.hand.push_back(revealed);
      shuffle(state_.prizes);
      trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
            "Looked at Prize cards and exchanged Heavy Ball for " + std::string(name(revealed)) + ".");
      return true;
    }

    // If no Basic was present, Heavy Ball resolves normally and is discarded.
    state_.discard.push_back(Card::HisuianHeavyBall);
    shuffle(state_.prizes);
    trace("PLAY ITEM", "R-HEAVYBALL-01; R-GAME-ITEM; P-KNOWLEDGE-01",
          "Looked at Prize cards; no Basic Pokémon was present, so Heavy Ball was discarded.");
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