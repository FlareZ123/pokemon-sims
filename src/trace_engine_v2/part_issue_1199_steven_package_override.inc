  bool issue_1199_preinspection_candidate() const {
    // This narrow K0 gate recognizes the reported opening schedule without assuming
    // any Prize or deck identity before Mysterious Treasure legally inspects the deck:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Hidden-information boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    return state_.turn == 1 && !scenario_.going_first &&
        scenario_.dci == DciProfile::NoDiscardControl &&
        scenario_.locks == LockMode::None && scenario_.max_turn >= 3 &&
        supporter_allowed() && !state_.manual_energy_used && !deck_seen_ &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        state_.bench.empty() && hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::Fire) > 0 && hand_count(Card::Grass) == 0 &&
        hand_count(Card::RegidragoVstar) == 0;
  }

  bool issue_1199_known_steven_package_available() const {
    // After Mysterious Treasure's legal inspection, K1 must prove every source in
    // the T1 Fire, T2 Grass plus evolution and Blender, T3 Grass schedule:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual attachment and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-discard-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    return deck_seen_ && supporter_allowed() && !state_.manual_energy_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::StevensResolve) > 0 && hand_count(Card::Fire) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
  }

  bool issue_1199_route_available() {
    if (!issue_1199_preinspection_candidate()) return false;

    // Project the exact legal Mysterious Treasure play with an isolated RNG copy.
    // The real state remains K0 unless that search proves the complete Steven package:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Knowledge model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    if (!projected.play_mysterious_treasure(false)) return false;
    return projected.issue_1199_known_steven_package_available();
  }

  bool play_issue_1199_steven_package() {
    if (!deck_seen_ || !supporter_allowed() || !state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->fire < 1 || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) < 2 ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0 ||
        !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
      return false;
    }

    // Fire has already moved from hand to the Active Regidrago V. Recheck the
    // remaining K1 package here without reusing the pre-attachment predicate:
    // Manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> found;
    for (int copy = 0; copy < 2; ++copy) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("Issue-1199 Steven Grass target disappeared");
      }
      found.push_back(Card::Grass);
    }
    if (!move_deck_to_hand(Card::BrilliantBlender)) {
      throw std::logic_error("Issue-1199 Steven Blender target disappeared");
    }
    found.push_back(Card::BrilliantBlender);
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven searches any three cards and ends the turn. Two Grass Energy plus the
    // one-copy Blender are the complete source-bounded package, so Quick Ball, Tapu
    // Lele-GX, Wonder Tag, Forest Seal Stone, and Star Alchemy remain unspent:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched the issue-1199 T3 package: " + join_cards(found) + ". Turn ended.");
    return true;
  }
