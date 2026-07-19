  bool known_turo_t4_steven_route_available() const {
    if (!deck_seen_ || state_.turn != 3 || scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 4 ||
        !supporter_allowed() || !state_.active ||
        state_.active->card != Card::DialgaGX ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::ProfessorTuro) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const bool prepared_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn &&
              pokemon.grass >= 1 && pokemon.fire >= 1;
        });
    if (!prepared_regidrago) return false;

    // Steven can reserve the missing VSTAR and final Grass, then ends T3. On T4 the
    // prior-turn Regidrago evolves, the manual attachment completes GGF, Quick Ball
    // discards the held Dragon during the strict-JIT ready turn, and the already held
    // Professor Turo removes the Basic Active so the completed VSTAR is promoted:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Core evolution, attachment, Supporter, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1030
    return hand_count(Card::RegidragoVstar) +
                deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) +
                deck_count_after_search_started(Card::Grass) > 0;
  }

  bool play_known_turo_t4_steven_route() {
    if (!known_turo_t4_steven_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> found;
    if (hand_count(Card::RegidragoVstar) == 0) {
      if (!move_deck_to_hand(Card::RegidragoVstar)) {
        throw std::logic_error("Known Turo route VSTAR target disappeared");
      }
      found.push_back(Card::RegidragoVstar);
    }
    const Pokemon* target = target_regi();
    if (target != nullptr && target->grass < 2 && hand_count(Card::Grass) == 0) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("Known Turo route Grass target disappeared");
      }
      found.push_back(Card::Grass);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches up to three cards, so selecting only the missing Evolution and
    // Energy pieces preserves the held Quick Ball, Dragon payload, and Turo promotion:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://github.com/FlareZ123/pokemon-sims/issues/1030
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the known T4 Turo promotion route: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  bool should_play_steven() const {
    return should_play_steven_issue1030_original() ||
        known_turo_t4_steven_route_available();
  }

  bool play_steven() {
    if (play_known_turo_t4_steven_route()) return true;
    return play_steven_issue1030_original();
  }
