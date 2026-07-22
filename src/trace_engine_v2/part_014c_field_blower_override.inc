  bool path_removal_unlocks_live_rule_box_route() {
    if (!rule_box_abilities_locked()) return false;

    // Evaluate Wonder Tag and an already-established Skyliner retreat line in the
    // exact counterfactual state after Path to the Peak leaves play. Restore the
    // public lock state before making the real Item decision:
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/issues/791
    const bool path_lock_removed_before = state_.path_lock_removed;
    state_.path_lock_removed = true;
    const bool wonder_tag_route = hand_count(Card::TapuLeleGX) > 0 &&
        !in_play(Card::TapuLeleGX) && bench_space() > 0 && needs_tapu_connector();
    const bool in_play_skyliner_route = need_active_vstar() &&
        can_free_retreat_with_latias();
    state_.path_lock_removed = path_lock_removed_before;

    // A held Latias ex plus a held VSTAR can preserve the final promotion axis
    // when a prior-turn Benched Regidrago V is ready to evolve. Path suppresses
    // Skyliner until it is removed:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool held_skyliner_route = hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 && !in_play(Card::LatiasEx) &&
        bench_space() > 0 && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && state_.active->card != Card::RegidragoV &&
        prior_turn_benched_regi;

    // Legacy Star is a Rule Box Pokémon Ability. Remove Path when an unused VSTAR
    // Power is already in play, or when a prior-turn Regidrago V has the held VSTAR
    // needed to evolve this turn while setup remains unresolved. The exact Legacy
    // Star selector still decides whether its recovery or discard-seven route is
    // worth using after Path leaves play:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool unresolved_setup = setup_axis_missing();
    const bool in_play_legacy_star_route = !state_.vstar_power_used && has_vstar() &&
        unresolved_setup;
    const bool prior_turn_active_regi = active_is_regi_v() && state_.active &&
        state_.active->entered_turn < state_.turn;
    const bool future_legacy_star_route = !state_.vstar_power_used &&
        prior_turn_active_regi && unresolved_setup &&
        hand_count(Card::RegidragoVstar) > 0;

    return wonder_tag_route || in_play_skyliner_route || held_skyliner_route ||
           in_play_legacy_star_route || future_legacy_star_route;
  }

  bool play_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0 ||
        !rule_box_abilities_locked() || !path_removal_unlocks_live_rule_box_route()) {
      return false;
    }

    // Field Blower may discard the Stadium in play. Spend it only when removing
    // the modeled Path to the Peak unlocks a currently live setup route, so a
    // discrete-value Stadium answer is preserved after setup is already complete:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://github.com/FlareZ123/pokemon-sims/issues/791
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);
    state_.path_lock_removed = true;
    trace("PLAY ITEM", "R-GAME-ITEM",
          "Field Blower discarded the modeled Path to the Peak; Rule Box Pokémon Abilities are available again.");
    return true;
  }
