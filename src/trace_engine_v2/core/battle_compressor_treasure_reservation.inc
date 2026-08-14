  bool issue_3545_reserve_mysterious_treasure_for_ready_turn(
      const bool permit_payload) const {
    // The BC + Powerglass + Steven package reserves Mysterious Treasure as the T2
    // same-turn Dragon discard outlet. Matchup-flex DCI may otherwise spend Treasure
    // before Crispin using an ordinary flexible cost, which removes the only proved
    // JIT outlet. Hold only during the pre-Supporter Item pass; once Crispin finishes
    // GGF and the later Item pass permits payload, Treasure is released normally.
    // Battle Compressor: https://api.pokemontcg.io/v2/cards/xy4-92
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Same-ready-turn JIT and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3545
    if (permit_payload || !strict_payload_timing() || state_.turn_ended ||
        state_.turn != 2 || !supporter_allowed() || item_locked() ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->double_dragon != 0 || state_.active->grass != 1 ||
        state_.active->fire < 1 || state_.active->tool != Tool::Powerglass ||
        !need_payload() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Crispin requires two different Basic Energy types to be present for this exact
    // package. K1 was established by Battle Compressor, so these are physical deck
    // checks rather than assumptions about the unknown next draw.
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return deck_seen_ &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
  }

  bool play_mysterious_treasure_issue3545_reserved(const bool permit_payload) {
    if (issue_3545_reserve_mysterious_treasure_for_ready_turn(permit_payload)) {
      return false;
    }
    return play_mysterious_treasure(permit_payload);
  }
