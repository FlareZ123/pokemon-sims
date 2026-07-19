  bool play_steven() {
    const int regi_before = hand_count(Card::RegidragoV);
    const int crispin_before = hand_count(Card::Crispin);
    const int gladion_before = hand_count(Card::Gladion);
    const bool all_vstar_copies_held =
        hand_count(Card::RegidragoVstar) == total_copies(Card::RegidragoVstar);
    const bool basic_active_needs_latias_bridge = state_.active &&
        is_basic(state_.active->card) &&
        state_.active->card != Card::RegidragoV &&
        !in_play(Card::LatiasEx);

    const bool played = play_steven_issue1002_empty_deck_original();
    if (!played) return false;

    const int regi_found = hand_count(Card::RegidragoV) - regi_before;
    const int crispin_found = hand_count(Card::Crispin) - crispin_before;
    const int gladion_found = hand_count(Card::Gladion) - gladion_before;
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
