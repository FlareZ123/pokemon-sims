  bool preserve_incomplete_tate_blender_line() const {
    const int supporters_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool only_tate_supporter_held =
        supporters_in_hand == hand_count(Card::TateLiza);

    // Tate & Liza can promote the Benched VSTAR and Brilliant Blender can supply
    // its current-turn payload, but Apex Dragon still requires GGF. When Tate is the
    // only held Supporter, retain both one-turn resources until that promoted VSTAR's
    // Energy axis is complete:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return supporter_allowed() && strict_payload_timing() && only_tate_supporter_held &&
           hand_count(Card::TateLiza) > 0 && need_active_vstar() && need_energy() &&
           has_live_blender_payload_line();
  }

  bool play_tate_switch() {
    if (preserve_incomplete_tate_blender_line()) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza because the promoted Regidrago VSTAR still lacks part of GGF.");
      return false;
    }
    return play_tate_switch_tate_blender_original();
  }

  bool play_tate_draw() {
    if (preserve_incomplete_tate_blender_line()) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza and Brilliant Blender instead of shuffling the incomplete promotion route.");
      return false;
    }
    return play_tate_draw_tate_blender_original();
  }
