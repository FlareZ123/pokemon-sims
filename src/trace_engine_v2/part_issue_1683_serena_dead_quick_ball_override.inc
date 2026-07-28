  bool issue_1683_rulebox_locked_quick_ball_is_serena_cost() const {
    if (scenario_.dci != DciProfile::StrictJit ||
        !rule_box_abilities_locked() || hand_count(Card::QuickBall) != 1 ||
        need_regi() || in_play_count(Card::RegidragoV) +
                           in_play_count(Card::RegidragoVstar) < 2 ||
        !in_play(Card::Oricorio)) {
      return false;
    }

    // Test the held Quick Ball against the exact public hand and K1 deck state.
    // When its printed discard cost and every setup-advancing Basic route both fail,
    // the singleton is setup-dead under the current Rule Box lock and is lower DCI
    // than Serena, Earthen Vessel, recovery, Energy, VSTAR, or a strict-JIT payload:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Core Supporter, Item, Ability, search, discard, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and earliest productive route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
    Engine projected = *this;
    return !projected.play_quick_ball(false);
  }

  bool play_serena(const bool allow_zero_draw_payload_completion = false) {
    if (play_serena_issue1683_original(allow_zero_draw_payload_completion)) {
      return true;
    }
    if (!supporter_allowed() || hand_count(Card::Serena) == 0 ||
        !issue_1683_rulebox_locked_quick_ball_is_serena_cost()) {
      return false;
    }

    // Serena must discard at least one card before drawing to five. In this
    // confirmed locked public state, Quick Ball has no productive legal route,
    // so spending it increases the current turn's setup probability while every
    // live singleton and unresolved axis remains protected:
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
    // Official Supporter, Item, Ability, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
    remove_one(state_.hand, Card::Serena);
    state_.discard.push_back(Card::Serena);
    discard_from_hand(Card::QuickBall, "Serena chosen discard", "R-SERENA-01");
    state_.supporter_used = true;
    while (state_.hand.size() < 5U && !state_.deck.empty()) draw_card("Serena");
    trace("PLAY SUPPORTER", "R-SERENA-01; R-GAME-SUPPORTER",
          "Used Serena draw mode after discarding setup-dead Quick Ball under Rule Box lock.");
    return true;
  }
