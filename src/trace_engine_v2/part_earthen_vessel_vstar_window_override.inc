  bool hold_earthen_vessel_for_next_turn_latias_jit() const {
    if (!strict_payload_timing() || scenario_.locks != LockMode::None ||
        state_.turn >= scenario_.max_turn || !state_.manual_energy_used ||
        !need_energy() || !need_payload() || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || !in_play(Card::LatiasEx) ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const Pokemon* setup_regi = target_regi();
    if (setup_regi == nullptr || setup_regi->card != Card::RegidragoVstar ||
        setup_regi->grass != 1 || setup_regi->fire != 1) {
      return false;
    }

    const auto possible_energy_in_deck = [this](const Card energy) {
      if (deck_seen_ || prizes_revealed_) return count_of(state_.deck, energy);
      const int public_copies = hand_count(energy) + count_of(state_.discard, energy) +
          attached_public_count(energy);
      return std::max(0, total_copies(energy) - public_copies);
    };
    const int grass_in_deck = possible_energy_in_deck(Card::Grass);
    const int basic_energy_in_deck =
        grass_in_deck + possible_energy_in_deck(Card::Fire);
    // K0 cannot distinguish Basic Energy in the deck from Basic Energy in Prizes.
    // Use fixed-list and public-zone possible counts until legal inspection permits
    // exact deck counts, so hidden placement cannot change an irreversible payload
    // discard decision:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv8-130
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core deck, Prize, Item, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed candidate: https://github.com/FlareZ123/pokemon-sims/issues/1471
    // Preserve Vessel and the held Dragon together when T2 has established GF and
    // Latias can promote the Benched VSTAR. On T3, Vessel discards that Dragon during
    // the strict-JIT ready turn, searches the final Grass, and leaves the Supporter
    // slot unused. Requiring two remaining Basic Energy cards keeps Vessel legally
    // playable after the intervening draw, even when that draw is the final Grass:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core attachment and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Strict JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1009
    return grass_in_deck >= 1 && basic_energy_in_deck >= 2;
  }

  bool play_earthen_vessel(const bool permit_payload) {
    // Earthen Vessel may still search Basic Energy with an ordinary discard cost.
    // Its strict-JIT Dragon-payload cost is permitted only when the Regidrago VSTAR
    // card axis can finish this turn or a permitted payload is already established:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/954
    if (permit_payload && hold_earthen_vessel_for_next_turn_latias_jit()) {
      return false;
    }
    const bool payload_cost_permitted = permit_payload &&
        (payload_ready() || blender_vstar_card_axis_can_finish_this_turn());
    return play_earthen_vessel_vstar_window_original(payload_cost_permitted);
  }
