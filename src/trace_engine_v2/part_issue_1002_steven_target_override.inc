  bool play_steven() {
    const int regi_before = hand_count(Card::RegidragoV);
    const int vstar_before = hand_count(Card::RegidragoVstar);
    const int crispin_before = hand_count(Card::Crispin);
    const int latias_before = hand_count(Card::LatiasEx);
    const int vessel_before = hand_count(Card::EarthenVessel);
    const int gladion_before = hand_count(Card::Gladion);
    const bool all_vstar_copies_held =
        hand_count(Card::RegidragoVstar) == total_copies(Card::RegidragoVstar);
    const bool basic_active_needs_latias_bridge = state_.active &&
        is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV &&
        !in_play(Card::LatiasEx);
    const Pokemon* setup_regi = target_regi();
    const bool held_zero_energy_regi = setup_regi != nullptr &&
        setup_regi->card == Card::RegidragoV && setup_regi->grass == 0 &&
        setup_regi->fire == 0;
    const bool held_vstar = hand_count(Card::RegidragoVstar) > 0;
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool zero_energy_latias_vessel_candidate =
        scenario_.dci == DciProfile::StrictJit && scenario_.locks == LockMode::None &&
        state_.turn + 2 <= scenario_.max_turn && basic_active_needs_latias_bridge &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && hand_count(Card::EarthenVessel) == 0 &&
        held_zero_energy_regi && held_vstar && held_payload;

    const bool played = play_steven_issue1002_empty_deck_original();
    if (!played) return false;

    const int regi_found = hand_count(Card::RegidragoV) - regi_before;
    const int vstar_found = hand_count(Card::RegidragoVstar) - vstar_before;
    const int crispin_found = hand_count(Card::Crispin) - crispin_before;
    const int latias_found = hand_count(Card::LatiasEx) - latias_before;
    const int vessel_found = hand_count(Card::EarthenVessel) - vessel_before;
    const int gladion_found = hand_count(Card::Gladion) - gladion_before;

    auto latias_in_deck = std::find(state_.deck.begin(), state_.deck.end(), Card::LatiasEx);
    auto vessel_in_deck =
        std::find(state_.deck.begin(), state_.deck.end(), Card::EarthenVessel);
    const bool energy_chain_remains =
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 1;
    const bool generic_fallback_displaced_complete_route =
        zero_energy_latias_vessel_candidate && regi_found == 1 && vstar_found == 1 &&
        crispin_found == 1 && latias_found == 0 && vessel_found == 0 &&
        latias_in_deck != state_.deck.end() && vessel_in_deck != state_.deck.end() &&
        energy_chain_remains;

    if (generic_fallback_displaced_complete_route) {
      // Steven may search any three cards. With a zero-Energy Regidrago V already in
      // play, a VSTAR and Dragon payload already held, and a non-Regidrago Basic Active,
      // Crispin + Latias ex + Earthen Vessel forms the complete observable T3 route:
      // T2 Crispin attaches one Energy and puts the other in hand for the manual
      // attachment; Latias supplies free Retreat; T3 Vessel discards the held Dragon,
      // searches the final Grass Energy, and preserves strict-JIT payload timing.
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core turn, attachment, evolution, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1009
      if (!remove_one(state_.hand, Card::RegidragoV) ||
          !remove_one(state_.hand, Card::RegidragoVstar)) {
        throw std::logic_error("Redundant Steven fallback targets disappeared");
      }
      *latias_in_deck = Card::RegidragoV;
      *vessel_in_deck = Card::RegidragoVstar;
      state_.hand.push_back(Card::LatiasEx);
      state_.hand.push_back(Card::EarthenVessel);

      // The delegated Steven resolution already randomized the remaining deck. Swap
      // only the selected card identities in place so deck size and random order remain
      // unchanged while the three legal targets become the complete route:
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://github.com/FlareZ123/pokemon-sims/issues/1009
      if (trace_ != nullptr && trace_->enabled && !trace_->lines.empty() &&
          trace_->lines.back().find("PLAY SUPPORTER") != std::string::npos &&
          trace_->lines.back().find("Searched up to 3 cards") != std::string::npos) {
        trace_->lines.pop_back();
      }
      trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Searched up to 3 cards: Crispin, Latias ex, Earthen Vessel. Turn ended.");
      return true;
    }

    const auto gladion_in_deck =
        std::find(state_.deck.begin(), state_.deck.end(), Card::Gladion);
    const bool known_prized_latias_bridge = prizes_known() &&
        prize_count_after_reveal(Card::LatiasEx) > 0 &&
        gladion_in_deck != state_.deck.end();

    if (!all_vstar_copies_held || !basic_active_needs_latias_bridge ||
        regi_found != 1 || crispin_found < 2 || gladion_found != 0 ||
        !known_prized_latias_bridge) {
      return true;
    }

    // Steven's Resolve may search any three cards, so an unavailable Regidrago VSTAR
    // must not consume a target slot and a second Crispin must not duplicate a card
    // already selected. After Steven legally establishes K1, Gladion is the live bridge
    // to the prized Latias ex that supplies a non-Regidrago Basic Active's no-Retreat route:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1002
    if (!remove_one(state_.hand, Card::Crispin)) {
      throw std::logic_error("Duplicate Steven Crispin target disappeared");
    }
    *gladion_in_deck = Card::Crispin;
    state_.hand.push_back(Card::Gladion);

    // The original Steven resolution already randomized the remaining deck. Gladion's
    // randomized slot becomes the unselected Crispin's slot, preserving a shuffled
    // remaining deck while correcting only the three legal search targets:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/issues/1002
    if (trace_ != nullptr && trace_->enabled && !trace_->lines.empty() &&
        trace_->lines.back().find("PLAY SUPPORTER") != std::string::npos &&
        trace_->lines.back().find("Searched up to 3 cards") != std::string::npos) {
      trace_->lines.pop_back();
    }
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01",
          "Searched up to 3 cards: Regidrago V, Crispin, Gladion. Turn ended.");
    return true;
  }
