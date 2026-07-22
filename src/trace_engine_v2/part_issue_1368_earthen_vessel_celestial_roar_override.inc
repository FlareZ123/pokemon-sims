  bool earthen_vessel_preserves_grass_for_immediate_celestial_roar() const {
    // Earthen Vessel may search for up to two Basic Energy. On this public T1
    // graph, Fire is the only deterministic search target: two held Grass cards
    // already cover the T1 attachment and the complementary T2 attachment after
    // any Celestial Roar Energy hit. Taking a surplus Grass would only lower the
    // randomized deck's Energy density before the attack:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // No-future-card-oracle boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1368
    return state_.turn == 1 && !scenario_.going_first &&
        state_.turn < scenario_.max_turn && !state_.turn_ended &&
        !state_.manual_energy_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        hand_count(Card::Grass) >= 2 && hand_count(Card::Fire) == 0 &&
        might_be_unseen(Card::Grass) && might_be_unseen(Card::Fire) &&
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload);
  }

  bool play_earthen_vessel(const bool permit_payload) {
    if (!earthen_vessel_preserves_grass_for_immediate_celestial_roar()) {
      return play_earthen_vessel_issue1368_original(permit_payload);
    }

    // Reuse every active legality boundary before applying the narrower optional
    // target choice. The special graph has no held payload, so the existing DCI
    // selector chooses the same ordinary printed cost as the wrapped action:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Trainer-effect legality ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1368
    if (item_locked() || hand_count(Card::EarthenVessel) == 0 ||
        !deck_search_available() || !earthen_vessel_has_legal_target()) {
      return false;
    }
    const auto cost = choose_discard(false, true, true, Card::EarthenVessel);
    if (!cost) return false;

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    discard_from_hand(*cost, "Earthen Vessel cost", "R-EV-01");
    record_deck_search_knowledge("Earthen Vessel");

    std::vector<Card> searched;
    // Search Fire after legal inspection when it exists. If every Fire is outside
    // the deck, selecting one Grass remains a legal up-to-one fallback without
    // inventing a hidden Fire target:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // K1 deck inspection: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    if (move_deck_to_hand(Card::Fire)) {
      searched.push_back(Card::Fire);
    } else if (move_deck_to_hand(Card::Grass)) {
      searched.push_back(Card::Grass);
    }
    shuffle(state_.deck);
    trace("Earthen Vessel", "R-EV-01; R-GAME-ITEM; P-COMPRESS-01",
          "Searched up to 2 Basic Energy: " + join_cards(searched) + ".");
    return true;
  }
