  bool issue_2271_surplus_regidrago_v_treasure_route_available() const {
    if (scenario_.dci != DciProfile::StrictJit || !prizes_known() ||
        item_locked() || !supporter_allowed() || state_.manual_energy_used ||
        !active_is_vstar() || state_.active->grass != 1 ||
        state_.active->fire != 1 || !need_energy() || !need_payload() ||
        need_regi() || need_vstar() || need_active_vstar() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoV) == 0 || bench_space() == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        hand_count(Card::Arven) > 0 || hand_count(Card::EarthenVessel) > 0 ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0) {
      return false;
    }
    const bool separate_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(),
        [](const Card card) { return card != Card::RegidragoV && is_payload(card); });
    if (!separate_payload) return false;

    // The Active Regidrago VSTAR already satisfies the Basic, evolution, and Active
    // axes, so the held extra Regidrago V is route-surplus. K1 proves Treasure can
    // find Tapu Lele-GX, Wonder Tag can find Arven, and Arven can find Earthen Vessel;
    // Vessel then discards the separate held Dragon on the current ready turn while
    // searching Grass for the unused manual attachment. Spending the surplus Basic
    // on Treasure therefore preserves every required downstream axis.
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Ability, Supporter, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2271
    return true;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!issue_2271_surplus_regidrago_v_treasure_route_available()) {
      return play_mysterious_treasure_issue2271_original(permit_payload);
    }

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::RegidragoV,
                           "Mysterious Treasure issue-2271 surplus Regidrago V cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-2271 surplus Regidrago V disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure issue-2271 Tapu route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("projected issue-2271 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          "Spent surplus Regidrago V and searched Tapu Lele-GX for Wonder Tag -> Arven -> Earthen Vessel.");
    return true;
  }
