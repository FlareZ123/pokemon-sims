  bool issue_1199_preinspection_candidate() const {
    // This narrow K0 gate recognizes the reported opening schedule without assuming
    // any Prize or deck identity before Mysterious Treasure legally inspects the deck:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Hidden-information boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    if (state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || state_.manual_energy_used || deck_seen_ ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        !state_.bench.empty() || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Fire) == 0 || hand_count(Card::Grass) > 0 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        !might_be_unseen(Card::RegidragoVstar)) {
      return false;
    }

    // K0 may prove only that Treasure has a legal observable cost and that the fixed
    // list still permits a VSTAR target. The package identities remain unknown until
    // the real Treasure search establishes K1:
    // Mysterious Treasure discard and search text: https://api.pokemontcg.io/v2/cards/sm6-113
    // Legal search and hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    return choose_discard(can_play_payload_this_turn(), true, true,
                          Card::MysteriousTreasure).has_value();
  }

  bool issue_1199_known_package_after_treasure() const {
    // After Mysterious Treasure's legal inspection, K1 must prove every source in
    // the Fire, Grass, Grass, VSTAR, and Blender schedule:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual attachment and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-discard-control policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    return deck_seen_ && supporter_allowed() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
  }

  bool issue_1199_route_available() const {
    // The early turn block is admitted only to perform the legal information action.
    // Steven remains unselected until the actual Treasure resolution proves K1:
    // Knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    return issue_1199_preinspection_candidate();
  }

  bool play_issue_1199_steven_package() {
    if (!issue_1199_known_package_after_treasure() ||
        !state_.manual_energy_used || !state_.active ||
        state_.active->fire < 1) {
      return false;
    }

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
    // Lele-GX, Wonder Tag, Forest Seal Stone, and Star Alchemy remain unspent on T1:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched the issue-1199 T3 package: " + join_cards(found) + ". Turn ended.");
    return true;
  }

  bool play_issue_1199_route_after_attachment() {
    if (state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::None || !state_.manual_energy_used ||
        deck_seen_ || !state_.active ||
        state_.active->card != Card::RegidragoV || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Grass) > 0 || hand_count(Card::RegidragoVstar) > 0) {
      return false;
    }

    // Resolve the legal K0 information action first. If K1 does not prove both Grass
    // copies, Blender, payload, and VSTAR access, leave Steven unused and continue the
    // ordinary Supporter policy from the now-known state:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Core Item, attachment, Supporter, and turn-end procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
    if (!play_mysterious_treasure(false)) return false;
    return play_issue_1199_steven_package();
  }
