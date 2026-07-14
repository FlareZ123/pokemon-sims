  void choose_supporter() {
    const int supporters_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool only_tate_supporter_held =
        supporters_in_hand == hand_count(Card::TateLiza);

    // Tate & Liza can promote the Benched VSTAR and Brilliant Blender can supply
    // its current-turn payload, but Apex Dragon still requires GGF. When Tate is the
    // only held Supporter, retain both one-turn resources until that promoted VSTAR's
    // Energy axis is complete instead of spending Tate on its draw mode:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool preserve_incomplete_tate_blender_line = supporter_allowed() &&
        strict_payload_timing() && only_tate_supporter_held &&
        hand_count(Card::TateLiza) > 0 && need_active_vstar() && need_energy() &&
        has_live_blender_payload_line();
    if (preserve_incomplete_tate_blender_line) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza and Brilliant Blender until the intended Benched Regidrago VSTAR can complete GGF.");
      return;
    }

    choose_supporter_tate_blender_original();
  }
