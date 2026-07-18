  bool steven_should_yield_to_crispin_mysterious_route() const {
    const bool exact_lock_window = scenario_.locks == LockMode::TurnTwoItem &&
        state_.turn == 1 && !scenario_.going_first && !item_locked();
    const bool active_regi_can_attack = state_.active &&
        state_.active->card == Card::RegidragoV && !state_.manual_energy_used;
    const bool inspected_vstar_and_crispin_targets = deck_seen_ &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0;
    const bool payable_mysterious_route = hand_count(Card::MysteriousTreasure) > 0 &&
        is_legal_cost_available(false, true);

    // Wonder Tag has already established K1. Suppress Steven only when the current
    // public state proves a payable Mysterious Treasure route to Regidrago VSTAR and
    // Crispin can power the Active Regidrago V while the manual attachment remains.
    // Steven would end the turn and delay the repository's earliest-ready objective:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/774
    return exact_lock_window && active_regi_can_attack &&
        inspected_vstar_and_crispin_targets && payable_mysterious_route &&
        crispin_can_advance_energy_axis();
  }

  bool should_play_steven() const {
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        late_steven_is_only_basic_connector() ||
        late_steven_has_known_t3_compression_route();
  }
