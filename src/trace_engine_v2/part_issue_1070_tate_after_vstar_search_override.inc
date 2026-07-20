  bool tate_draw_after_active_vstar_evolution_route() const {
    // Under strict JIT, a prior-turn Active Regidrago V that already has GGF cannot
    // improve its Energy or ready-turn payload axes with Celestial Roar. When Star
    // Alchemy has placed Regidrago VSTAR in hand during Item lock, evolve before
    // Tate & Liza replaces the hand with five cards:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Evolution, Supporter, search, shuffle, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest legal route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1162
    return supporter_allowed() && strict_payload_timing() && item_locked() &&
           hand_count(Card::TateLiza) > 0 &&
           hand_count(Card::RegidragoVstar) > 0 && state_.active &&
           state_.active->card == Card::RegidragoV &&
           state_.active->entered_turn < state_.turn &&
           state_.active->grass >= 2 && state_.active->fire >= 1;
  }

  bool tate_switch_after_held_vstar_evolution_route() const {
    const bool payload_in_known_deck = !deck_seen_ ||
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || item_locked() ||
        !state_.active || state_.active->card == Card::RegidragoVstar ||
        !payload_in_known_deck || can_free_retreat_with_latias()) {
      return false;
    }

    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
          pokemon.fire >= 1;
    });
  }

  void choose_supporter() {
    if (tate_draw_after_active_vstar_evolution_route()) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1162-tate-after-active-vstar-search", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; R-FSS-02; R-RVS-01; P-JIT-01; P-COMPRESS-01",
            "Retained Tate & Liza until the held Regidrago VSTAR evolves the prior-turn GGF Active.");
      }
      return;
    }
    if (tate_switch_after_held_vstar_evolution_route()) {
      // Star Alchemy has already placed Regidrago VSTAR in hand. Preserve Tate & Liza
      // through the current fixed-point pass so the prior-turn GGF Basic evolves first;
      // Tate's switch mode then promotes it and the held Brilliant Blender supplies the
      // current-turn JIT payload. The concrete public state is sufficient even before
      // generic VSTAR-axis helpers recognize the unevolved Benched attacker:
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Supporter, evolution, switching, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Strict and matchup-flex JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1070
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1070-tate-after-vstar-search", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; R-RVS-01; P-COMPRESS-01",
            "Retained Tate & Liza until the held Regidrago VSTAR evolves the prepared Benched Basic.");
      }
      return;
    }
    choose_supporter_issue1070_original();
  }
