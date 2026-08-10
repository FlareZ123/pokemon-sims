  std::optional<Card> issue_1871_searchable_payload() const {
    for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX,
                            Card::Appletun}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1871_treasure_serena_payload_route_available() const {
    // Once a legal deck or full Prize inspection establishes K1, the simulator may
    // choose a known unprized Dragon rather than gamble Legacy Star's top-seven
    // discard. The route spends one surplus Grass on Mysterious Treasure, then
    // Serena discards the searched Dragon after the manual attachment completes GGF.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, attachment, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, Supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1871
    const bool same_turn_jit =
        scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::MatchupFlexJit;  // Same ready-turn payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2741
    return same_turn_jit && scenario_.going_first && state_.turn == 3 &&
        prizes_known() && !item_locked() && supporter_allowed() &&
        !state_.manual_energy_used && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 && need_payload() &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::Serena) > 0 && hand_count(Card::Grass) >= 2 &&
        issue_1871_searchable_payload().has_value();
  }

  bool complete_issue_1871_treasure_serena_payload_route() {
    if (!issue_1871_treasure_serena_payload_route_available()) return false;

    const std::optional<Card> payload = issue_1871_searchable_payload();
    if (!payload.has_value()) {
      throw std::logic_error("Issue-1871 payload target disappeared");
    }

    // Complete GGF first. Mysterious Treasure then spends the second Grass, searches
    // the known Dragon payload, and Serena discards that Dragon before drawing to
    // five. The payload therefore enters discard on the exact strict-JIT ready turn.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dragon-type Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Supporter, attachment, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1871
    // Use the exact Grass attachment instead of the generic attachment dispatcher.
    // The dispatcher may replay an unrelated held Brilliant Blender after GGF is
    // completed, which can consume the selected deck payload before Treasure.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official once-per-turn attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1871 Grass attachment disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
          "Grass Energy manually to Regidrago VSTAR before the Treasure-Serena finish.");

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::Grass, "Mysterious Treasure issue-1871 surplus Grass cost",
                           "R-MT-01; P-DCI-01")) {
      throw std::logic_error("Issue-1871 Treasure cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1871 payload route");
    if (!move_deck_to_hand(*payload)) {
      throw std::logic_error("Issue-1871 searched payload disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01",
          "Mysterious Treasure spent surplus Grass and searched the known strict-JIT Dragon payload.");

    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    if (!discard_from_hand(*payload, "Serena issue-1871 current-turn Dragon payload",
                           "R-SERENA-01; P-JIT-01")) {
      throw std::logic_error("Issue-1871 Serena payload disappeared");
    }
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER; P-JIT-01",
          "Serena discarded the searched Dragon payload and drew until five cards.");

    return active_is_vstar() && !need_energy() && !need_payload();
  }
