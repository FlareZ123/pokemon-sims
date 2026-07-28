  bool complete_issue_1674_turo_late_promotion() {
    if (state_.turn_ended || state_.turn != scenario_.max_turn ||
        !need_active_vstar() || need_regi() || need_vstar() ||
        need_energy() || need_payload() ||
        hand_count(Card::ProfessorTuro) == 0 ||
        !benched_vstar_promotion_ready()) {
      return false;
    }

    // The Benched Regidrago VSTAR already has GGF, and Earthen Vessel paid
    // Dragapult ex as the strict-JIT payload this turn. Professor Turo's Scenario
    // returns the Basic Active Pokémon to hand, which immediately requires a
    // Benched replacement; promoting the complete VSTAR resolves the sole remaining
    // Active-position axis without another discard, attachment, or search:
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter and Active-replacement procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route and Supporter contention: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1674
    return play_turo_active_promotion_route();
  }

  void run_turn() {
    run_turn_issue1674_original();
    if (complete_issue_1674_turo_late_promotion()) {
      trace("POLICY", "P-AXIS-01; R-GAME-SUPPORTER",
            "Completed the deadline Professor Turo promotion route: " + state_line());
    }
  }
