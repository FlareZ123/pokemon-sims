  bool tate_switch_after_held_vstar_evolution_route() const {
    if (!supporter_allowed() || hand_count(Card::TateLiza) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || item_locked() ||
        !need_active_vstar() || !need_payload() ||
        !payload_might_be_in_deck() || can_free_retreat_with_latias()) {
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
    if (tate_switch_after_held_vstar_evolution_route()) {
      // Star Alchemy has already placed Regidrago VSTAR in hand. Preserve Tate & Liza
      // through the current fixed-point pass so the prior-turn GGF Basic evolves first;
      // Tate's switch mode then promotes it and the held Brilliant Blender supplies the
      // strict-JIT payload. Draw mode would shuffle every deterministic route piece:
      // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Supporter, evolution, switching, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
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
