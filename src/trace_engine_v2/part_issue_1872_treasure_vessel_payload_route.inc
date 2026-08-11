  bool issue_1872_treasure_vessel_payload_route_available() const {
    // Public K1 makes the deck payload and Basic Energy counts deterministic. Spend
    // Chaotic Swell on Mysterious Treasure first, then use the searched Dragon as
    // Earthen Vessel's current-turn payload cost before attaching the searched Grass.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, attachment, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, connector ordering, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed route bug: https://github.com/FlareZ123/pokemon-sims/issues/1872
    // Confirmed profile-overfitting bug: https://github.com/FlareZ123/pokemon-sims/issues/2742
    return strict_payload_timing() && state_.turn == 4 && prizes_known() &&
        !item_locked() && !state_.manual_energy_used && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 && need_payload() &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::ChaoticSwell) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        treasure_route_searchable_payload().has_value();
  }

  bool complete_issue_1872_treasure_vessel_payload_route() {
    if (!issue_1872_treasure_vessel_payload_route_available()) return false;

    const std::optional<Card> payload = treasure_route_searchable_payload();
    if (!payload.has_value()) {
      throw std::logic_error("Issue-1872 payload target disappeared");
    }

    // Mysterious Treasure uses the setup-dead Stadium cost and searches the known
    // Dragon. Earthen Vessel immediately discards that Dragon, searches Grass, and
    // the manual attachment completes GGF on the same JIT-ready turn.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Appletun: https://api.pokemontcg.io/v2/cards/sv8-140
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, attachment, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI/JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1872 https://github.com/FlareZ123/pokemon-sims/issues/2742
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::ChaoticSwell,
                           "Mysterious Treasure issue-1872 Stadium cost",
                           "R-MT-01; P-DCI-01")) {
      throw std::logic_error("Issue-1872 Treasure cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-1872 payload route");
    if (!move_deck_to_hand(*payload)) {
      throw std::logic_error("Issue-1872 searched payload disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01",
          "Mysterious Treasure spent Chaotic Swell and searched the known Dragon payload.");

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*payload,
                           "Earthen Vessel issue-1872 current-turn Dragon payload",
                           "R-EV-01; P-JIT-01")) {
      throw std::logic_error("Issue-1872 Vessel payload disappeared");
    }
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM; P-COMPRESS-01",
                          "Earthen Vessel");
    // Attach the exact searched Grass. The generic attachment dispatcher may replay
    // an unrelated held Brilliant Blender after GGF, which is outside this proven
    // two-Item route and can alter its resource accounting.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Official once-per-turn attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    attach_treasure_route_grass("Issue-1872 Grass attachment disappeared");
    trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
          "Grass Energy manually to Regidrago VSTAR after the Treasure-Vessel payload route.");
    return active_is_vstar() && !need_energy() && !need_payload();
  }
