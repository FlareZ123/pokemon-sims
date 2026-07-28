  bool issue_1744_quick_ball_forretress_route_available() const {
    const bool prior_turn_pineco =
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      return pokemon.card == Card::Pineco &&
                             pokemon.entered_turn < state_.turn;
                    });
    const bool held_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool searchable_basic =
        std::any_of(state_.deck.begin(), state_.deck.end(), is_basic);

    // Quick Ball pays one current-turn Dragon payload, while the prior-turn Pineco
    // evolves normally and Exploding Energy supplies the only missing Grass. This
    // reaches the same T3 deadline without spending the three-card ACE SPEC route:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, evolution, Ability, attachment, Knock Out, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1744
    return secret_box_combo_enabled() && deck_seen_ && strict_payload_timing() &&
        state_.turn == 3 && scenario_.locks == LockMode::None && !item_locked() &&
        !state_.turn_ended && state_.active &&
        state_.active->card == Card::RegidragoVstar &&
        grass_needed() == 1 && fire_needed() == 0 && need_payload() &&
        prior_turn_pineco && in_play_count(Card::ForretressEx) == 0 &&
        ability_available_for_pokemon(Card::ForretressEx) &&
        hand_count(Card::ForretressEx) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::SecretBox) > 0 && held_payload && searchable_basic &&
        deck_count_after_search_started(Card::Grass) > 0;
  }

  bool play_issue_1744_quick_ball_forretress_route() {
    if (!issue_1744_quick_ball_forretress_route_available()) return false;

    const auto payload_it =
        std::find_if(state_.hand.begin(), state_.hand.end(), is_payload);
    const auto basic_it =
        std::find_if(state_.deck.begin(), state_.deck.end(), is_basic);
    if (payload_it == state_.hand.end() || basic_it == state_.deck.end()) {
      throw std::logic_error("Issue-1744 proved Quick Ball resources disappeared");
    }
    const Card payload = *payload_it;
    const Card basic = *basic_it;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(payload,
                           "Quick Ball current-turn payload cost",
                           "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1744 Quick Ball payload disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1744 route");
    if (!move_deck_to_hand(basic)) {
      throw std::logic_error("Issue-1744 Quick Ball Basic target disappeared");
    }
    shuffle(state_.deck);
    trace("QUICK BALL", "R-QB-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded " + std::string(name(payload)) +
              " as the current-turn payload and searched " +
              std::string(name(basic)) +
              "; preserved Secret Box and the Supporter action.");

    advance_forretress_combo();
    if (need_energy() || need_payload()) {
      throw std::logic_error("Issue-1744 proved Forretress route did not complete");
    }
    trace("POLICY", "R-QB-01; R-FORRETRESS-01; P-AXIS-01; P-COMPRESS-01",
          "Completed the one-cost Quick Ball and prior-turn Forretress route before Secret Box: " +
              state_line());
    return true;
  }

  void run_secret_box_turn_issue1744() {
    if (play_issue_1744_quick_ball_forretress_route()) return;
    run_secret_box_turn();
  }
