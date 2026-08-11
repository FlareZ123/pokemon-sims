  bool issue_1816_public_preflight() const {
    const bool registered_pineco_recipe =
        registered_recipe_count(Card::SecretBox) == 1 &&
        registered_recipe_count(Card::Pineco) == 2 &&
        registered_recipe_count(Card::ForretressEx) == 2;

    // Both registered JIT profiles require the Dragon payload on the ready turn:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2765
    return registered_pineco_recipe && secret_box_combo_enabled() &&
        strict_payload_timing() && scenario_.going_first && state_.turn == 2 &&
        state_.turn < scenario_.max_turn && !state_.turn_ended &&
        supporter_allowed() && !state_.manual_energy_used &&
        scenario_.locks != LockMode::FullItem &&
        scenario_.locks != LockMode::FullCombined &&
        scenario_.locks != LockMode::TurnTwoItem && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 1 && state_.active->fire == 0 &&
        hand_count(Card::Fire) > 0 && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        hand_count(Card::SecretBox) == 0 &&
        in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
  }

  bool issue_1816_targets_available_after_steven() const {
    if (!deck_seen_ ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0) {
      return false;
    }

    // Steven removes Regidrago VSTAR from the eligible search targets. Two more
    // Psychic or Dragon targets must remain so the mandatory T3 draw cannot
    // eliminate Mysterious Treasure's final legal target:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official draw, search, evolution, attachment, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, same-turn JIT, lock, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1816
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2765
    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    return treasure_targets >= 3;
  }

  bool play_issue_1816_direct_t3_route() {
    if (!issue_1816_public_preflight()) return false;

    // Project the legal Fire attachment and Steven deck inspection on a copy.
    // The route decision uses only the visible hand and board before Steven;
    // exact target identities are consulted only after the simulated search
    // establishes K1:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Official deck-search knowledge and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // No-future-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Original confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/1816
    Engine projected = *this;
    projected.trace_ = nullptr;
    remove_one(projected.state_.hand, Card::Fire);
    ++projected.state_.active->fire;
    projected.state_.manual_energy_used = true;
    remove_one(projected.state_.hand, Card::StevensResolve);
    projected.state_.discard.push_back(Card::StevensResolve);
    projected.state_.supporter_used = true;
    projected.state_.turn_ended = true;
    projected.record_deck_search_knowledge("Steven's Resolve");
    if (!projected.issue_1816_targets_available_after_steven()) return false;

    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
          "Start Pineco policy: " + state_line());
    remove_one(state_.hand, Card::Fire);
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Fire Energy manually to Regidrago V.");

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> selected;
    for (const Card card : {Card::RegidragoVstar, Card::Grass,
                            Card::MysteriousTreasure}) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    }
    if (selected.size() != 3U) {
      throw std::logic_error("Issue 1816 Steven targets disappeared after K1 projection");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-MT-01; P-AXIS-01; P-DCI-01",
          "Steven's Resolve banked Regidrago VSTAR, Grass Energy, and "
          "Mysterious Treasure for the direct next-turn same-turn-JIT route; "
          "Secret Box was preserved and the turn ended.");
    return true;
  }

  void run_secret_box_turn_issue1816() {
    if (play_issue_1816_direct_t3_route()) return;
    run_secret_box_turn_issue1744();
  }
