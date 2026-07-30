  std::optional<Card> issue_1874_searchable_payload() const {
    for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX,
                            Card::Appletun}) {
      if (is_payload(card) && deck_count_after_search_started(card) > 0) {
        return card;
      }
    }
    return std::nullopt;
  }

  int issue_1874_mysterious_target_count() const {
    int count = 0;
    // Mysterious Treasure may search every Psychic or Dragon Pokémon below. The
    // count proves that a legal second target remains after the first search.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Legal Dragon fallbacks: https://api.pokemontcg.io/v2/cards/sv8-140 https://api.pokemontcg.io/v2/cards/sv6-127
    // Prior target-classification fix: https://github.com/FlareZ123/pokemon-sims/issues/1253
    for (const Card card : {Card::RegidragoV, Card::RegidragoVstar,
                            Card::LatiasEx, Card::Oricorio,
                            Card::TapuLeleGX, Card::CrobatV,
                            Card::MegaDragonite, Card::Dragapult,
                            Card::GoodraVstar, Card::DialgaGX,
                            Card::Appletun, Card::Dipplin}) {
      count += deck_count_after_search_started(card);
    }
    return count;
  }

  bool issue_1874_duplicate_treasure_payload_route_available() const {
    // The held Fire and unused manual attachment complete GGF. Earthen Vessel is
    // therefore route-replaced and can pay the first Treasure. That Treasure
    // searches a Dragon, and the duplicate Treasure discards it during the same
    // strict-JIT turn before searching a second legal target.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, cost, search, discard, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1874
    return scenario_.dci == DciProfile::StrictJit && state_.turn == 3 &&
        prizes_known() && !item_locked() && !state_.manual_energy_used &&
        deck_search_available() && active_is_vstar() && state_.active &&
        state_.active->grass == 2 && state_.active->fire == 0 &&
        need_payload() && hand_count(Card::Fire) > 0 &&
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::EarthenVessel) > 0 &&
        issue_1874_searchable_payload().has_value() &&
        issue_1874_mysterious_target_count() >= 2;
  }

  bool complete_issue_1874_duplicate_treasure_payload_route() {
    if (!issue_1874_duplicate_treasure_payload_route_available()) return false;

    const std::optional<Card> payload = issue_1874_searchable_payload();
    if (!payload.has_value()) {
      throw std::logic_error("Issue-1874 payload target disappeared");
    }

    // Attach the already-held Fire exactly. Earthen Vessel's Energy search has no
    // remaining setup value in this public state, so it becomes the first Item cost.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Official once-per-turn Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Route-replacement DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1874
    if (!remove_one(state_.hand, Card::Fire)) {
      throw std::logic_error("Issue-1874 held Fire disappeared");
    }
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
          "Fire Energy manually to Regidrago VSTAR before the duplicate-Treasure payload bridge.");

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::EarthenVessel,
                           "Mysterious Treasure issue-1874 route-replaced Vessel cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1874 first Treasure cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1874 payload search");
    if (!move_deck_to_hand(*payload)) {
      throw std::logic_error("Issue-1874 searched payload disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-COMPRESS-01",
          "First Mysterious Treasure spent route-replaced Earthen Vessel and searched the known Dragon payload.");

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*payload,
                           "Mysterious Treasure issue-1874 current-turn Dragon payload",
                           "R-MT-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1874 second Treasure payload disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1874 second search");
    const Card second_target = fallback_mysterious_target_after_search_started();
    if (deck_count_after_search_started(second_target) <= 0 ||
        !move_deck_to_hand(second_target)) {
      throw std::logic_error("Issue-1874 second Treasure target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Second Mysterious Treasure discarded the searched Dragon as the current-turn payload and searched another legal target.");

    return active_is_vstar() && !need_energy() && !need_payload();
  }
