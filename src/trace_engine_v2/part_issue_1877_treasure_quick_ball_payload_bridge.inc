  std::optional<Card> issue_1877_deck_payload() const {
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1877_basic_target() const {
    for (const Card card : {Card::RegidragoV, Card::LatiasEx,
                            Card::Oricorio, Card::TapuLeleGX,
                            Card::CrobatV, Card::MawileGX,
                            Card::DialgaGX}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1877_treasure_quick_ball_payload_bridge_available() const {
    return scenario_.dci == DciProfile::StrictJit && state_.turn == 3 &&
        deck_seen_ && !item_locked() && active_is_vstar() &&
        state_.active->grass >= 2 && state_.active->fire >= 1 &&
        need_payload() && hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        issue_1877_deck_payload().has_value() &&
        issue_1877_basic_target().has_value();
  }

  bool complete_issue_1877_treasure_quick_ball_payload_bridge() {
    if (!issue_1877_treasure_quick_ball_payload_bridge_available()) return false;

    const Card payload = *issue_1877_deck_payload();
    const Card basic = *issue_1877_basic_target();

    // GGF is already complete, so Earthen Vessel's Energy role is fully
    // route-replaced. Mysterious Treasure may spend Vessel and search a Dragon;
    // Quick Ball then discards that searched Dragon as the current-turn Apex
    // Dragon payload and searches any legal Basic, which may remain in hand.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Example Dragon payloads: https://api.pokemontcg.io/v2/cards/me2pt5-152 https://api.pokemontcg.io/v2/cards/sv6-130 https://api.pokemontcg.io/v2/cards/sm5-100
    // Official Item, discard, search, and shuffle procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector composition, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Corrected confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::EarthenVessel,
                           "Mysterious Treasure issue-1877 route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1877 Earthen Vessel cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1877 payload bridge");
    if (!move_deck_to_hand(payload)) {
      throw std::logic_error("Issue-1877 Dragon payload target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Mysterious Treasure spent route-replaced Earthen Vessel and searched " +
              std::string(name(payload)) + ".");

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(payload,
                           "Quick Ball issue-1877 current-turn payload cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1877 searched Dragon payload disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1877 payload bridge");
    if (!move_deck_to_hand(basic)) {
      throw std::logic_error("Issue-1877 Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Quick Ball discarded the searched Dragon payload and searched " +
              std::string(name(basic)) + "; the Basic may remain in hand.");
    return payload_ready();
  }

