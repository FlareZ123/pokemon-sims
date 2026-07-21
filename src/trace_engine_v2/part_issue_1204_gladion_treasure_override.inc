  bool issue_1204_blind_gladion_missing_regi_admitted() const {
    if (prizes_known() || !supporter_allowed() || hand_count(Card::Gladion) == 0 ||
        !need_regi() || bench_space() == 0 || hand_count(Card::RegidragoV) > 0) {
      return false;
    }

    const bool payable_direct_search = !item_locked() &&
        is_legal_cost_available(can_play_payload_this_turn(), true) &&
        (hand_count(Card::QuickBall) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0);
    const bool held_arven_route = !item_locked() && hand_count(Card::Arven) > 0;

    // Mirror the existing blind missing-Basic admission. The decision to play
    // Gladion uses only the public hand and field, before any Prize identity is read:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    return !payable_direct_search && hand_count(Card::UltraBall) == 0 &&
        !held_arven_route;
  }

  bool issue_1204_prized_treasure_route_after_legal_reveal() const {
    if (!issue_1204_blind_gladion_missing_regi_admitted()) return false;

    Engine projected = *this;
    if (!remove_one(projected.state_.hand, Card::Gladion)) return false;
    projected.state_.supporter_used = true;
    projected.prizes_revealed_ = true;

    if (projected.item_locked() || projected.bench_space() == 0 ||
        projected.prize_count_after_reveal(Card::RegidragoV) > 0 ||
        projected.prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        projected.deck_count_after_search_started(Card::RegidragoV) == 0) {
      return false;
    }

    projected.state_.hand.push_back(Card::MysteriousTreasure);
    // After Gladion legally reveals every Prize, the searched Item and its remaining
    // discard cost are public. Require an actual cost after Gladion leaves the hand:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core Supporter, Item, discard, search, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Immediate missing-axis priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    return projected.choose_discard(projected.can_play_payload_this_turn(), true,
                                    true, Card::MysteriousTreasure).has_value();
  }

  bool play_gladion() {
    if (!issue_1204_prized_treasure_route_after_legal_reveal()) {
      return play_gladion_issue1204_original();
    }

    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(),
                                 Card::MysteriousTreasure);
    if (prize == state_.prizes.end()) {
      throw std::logic_error("Issue-1204 revealed Mysterious Treasure disappeared");
    }
    *prize = Card::Gladion;
    state_.hand.push_back(Card::MysteriousTreasure);
    shuffle(state_.prizes);

    // The revealed Treasure is an immediate, payable Regidrago V connector. It
    // precedes the inert Dipplin fallback while direct prized Regidrago recovery and
    // every other established Gladion target remain delegated to the prior policy:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Dipplin: https://api.pokemontcg.io/v2/cards/sv6-127
    // Core Supporter and Prize exchange procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1204
    trace("PLAY SUPPORTER", "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Looked at Prize cards and exchanged Gladion for Mysterious Treasure to establish the missing Regidrago V route.");
    return true;
  }
