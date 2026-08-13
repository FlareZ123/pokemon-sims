  bool issue_1646_vessel_burnet_finish_visible() const {
    const Pokemon* active = state_.active ? &*state_.active : nullptr;
    if (active == nullptr || active->card != Card::RegidragoVstar) return false;
    const auto missing_energy =
        issue_2437_apex_completing_basic_from_known_deck(*active);

    // DDE changes the physical Apex completion only. Route admission follows live
    // Item/search, Supporter, K1, attachment, payload, and same-ready-turn JIT state.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Item and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L440
    // K1, JIT, locks, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3400
    return strict_payload_timing() && !item_locked() &&
        deck_search_available() && state_.turn <= scenario_.max_turn &&
        prizes_known() && supporter_allowed() && !state_.manual_energy_used &&
        missing_energy.has_value() && need_payload() &&
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        payload_might_be_in_deck() &&
        issue_1866_vessel_burnet_cost().has_value();
  }

  bool play_issue_1646_vessel_burnet_finish() {
    if (!issue_1646_vessel_burnet_finish_visible()) return false;
    const Card cost = *issue_1866_vessel_burnet_cost();
    const auto missing_energy =
        issue_2437_apex_completing_basic_from_known_deck(*state_.active);
    if (!missing_energy) throw std::logic_error("Vessel Basic target disappeared");

    // Earthen Vessel pays its printed discard cost and searches the one Basic that
    // completes Apex; Burnet then establishes the Dragon payload in the same turn.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L440
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3400
    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(cost, "Earthen Vessel cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Vessel cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");
    if (!move_deck_to_hand(*missing_energy)) {
      throw std::logic_error("Basic Energy target disappeared");
    }
    shuffle(state_.deck);
    trace("Earthen Vessel",
          "R-EV-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-DCI-01; P-JIT-01; P-COMPRESS-01",
          "Searched the one-Basic DDE Apex completion for the Burnet finish.");
    if (!play_professor_burnet()) {
      throw std::logic_error("Professor Burnet route disappeared");
    }
    return true;
  }
