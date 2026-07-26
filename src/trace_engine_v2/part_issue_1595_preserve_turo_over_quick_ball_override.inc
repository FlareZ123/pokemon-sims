  bool play_quick_ball(const bool permit_payload) {
    const bool issue_1595_hold_quick_ball_for_turo =
        scenario_.dci == DciProfile::MatchupFlexJit && state_.turn == 4 &&
        state_.active && state_.active->card == Card::TapuLeleGX &&
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [](const Pokemon& pokemon) {
                      return pokemon.card == Card::RegidragoVstar &&
                             pokemon.grass >= 2 && pokemon.fire >= 1;
                    }) &&
        hand_count(Card::ProfessorTuro) > 0 &&
        (hand_count(Card::BrilliantBlender) > 0 || payload_ready()) &&
        hand_count(Card::QuickBall) > 0 && !state_.supporter_used;
    if (issue_1595_hold_quick_ball_for_turo) {
      // Quick Ball is redundant because Professor Turo already solves the Active
      // axis without a discard cost. Preserve Turo, let Blender establish the
      // current-turn payload, then return Tapu Lele-GX and promote Regidrago VSTAR:
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core Item, Supporter, return, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1595
      trace("HOLD QUICK BALL", "R-QB-01; P-DCI-01; P-COMPRESS-01; P-CONNECTOR-01",
            "Held redundant Quick Ball because Professor Turo already completes the post-Blender Active route.");
      return false;
    }
    return play_quick_ball_issue1595_original(permit_payload);
  }
