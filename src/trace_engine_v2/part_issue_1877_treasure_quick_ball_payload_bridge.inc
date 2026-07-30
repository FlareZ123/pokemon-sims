  std::optional<Card> issue_1877_treasure_payload_target() const {
    // Use a fixed public priority after K1 so the route never depends on hidden
    // deck order. Every listed card is a permitted Apex Dragon payload and a
    // legal Mysterious Treasure target:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // K1 and hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1877_quick_ball_basic_target() const {
    // Quick Ball requires a Basic search target. The searched Basic can remain
    // in hand because discarding the searched Dragon already completes the sole
    // missing strict-JIT axis:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Official Item, discard-cost, search, and shuffle procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
    for (const Card card : {Card::RegidragoV, Card::LatiasEx,
                            Card::TapuLeleGX, Card::Oricorio,
                            Card::DialgaGX, Card::CrobatV,
                            Card::MawileGX, Card::Pineco}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1877_treasure_quick_ball_payload_bridge_available() const {
    // The Active attacker already satisfies Apex Dragon's GGF cost. Earthen
    // Vessel has no remaining setup role and is DCI 1 for Mysterious Treasure.
    // Treasure searches a permitted Dragon, then Quick Ball discards that Dragon
    // during the ready turn and searches a legal Basic. Bench space is irrelevant
    // because the searched Basic may remain in hand:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard-cost, search, shuffle, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector composition, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1877
    return !secret_box_combo_enabled() && deck_seen_ && strict_payload_timing() &&
        scenario_.dci == DciProfile::StrictJit && state_.turn == 3 &&
        !state_.turn_ended && !item_locked() && active_is_vstar() &&
        !need_energy() && need_payload() &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        issue_1877_treasure_payload_target().has_value() &&
        issue_1877_quick_ball_basic_target().has_value();
  }

  bool complete_issue_1877_treasure_quick_ball_payload_bridge() {
    if (!issue_1877_treasure_quick_ball_payload_bridge_available()) return false;

    const Card payload = *issue_1877_treasure_payload_target();
    const Card basic = *issue_1877_quick_ball_basic_target();

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::EarthenVessel,
                           "Mysterious Treasure route-replaced Energy-search cost",
                           "R-MT-01; P-KNOWLEDGE-01; P-DCI-01")) {
      throw std::logic_error("Issue-1877 Earthen Vessel cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1877 bridge");
    if (!move_deck_to_hand(payload)) {
      throw std::logic_error("Issue-1877 Treasure payload target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM",
          "R-MT-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Mysterious Treasure discarded route-replaced Earthen Vessel and searched " +
              std::string(name(payload)) + " for the Quick Ball payload bridge.");

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(payload, "Quick Ball current-turn Dragon payload cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1877 searched Dragon payload disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1877 bridge");
    if (!move_deck_to_hand(basic)) {
      throw std::logic_error("Issue-1877 Quick Ball Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("QUICK BALL",
          "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Discarded searched " + std::string(name(payload)) +
              " as the current-turn payload and searched " +
              std::string(name(basic)) + "; the Basic remains in hand.");

    if (need_energy() || need_payload() || !active_is_vstar()) {
      throw std::logic_error("Issue-1877 proved bridge did not complete readiness");
    }
    return true;
  }
